/*================================================================================
    Lexing for PLY file headers.

notes:
================================================================================*/
%option noyywrap nodefault
%s Magic
%s Versioning
%s Elements
%s ElementName
%s ElementCount
%s Properties
%s PropertyListCountType
%s PropertyType
%s PropertyName

%{
#include <stdbool.h>
#include "helper_definitions.h"
#include "ply.h"

#define ERROR 0
#define END_HEADER 1

const bool g_debugging = true;

// Add the element the lexing state machine has built up so far.
static void add_element(void);
// Add the property.
static void add_property(void);
static void _lex_error(char *string);
static void debug(char *debug_string);

// Make sure when end_header is reached that at least one element is declared.
static bool g_at_least_one_element = false;

// Build up the PLY object while lexing, which will be copied over.
static PLY g_ply;
// Build up new elements and properties, to be allocated memory and copied over for the PLY object.
static PLYElement g_ply_element;
static PLYElement *g_last_ply_element; // this is used when a property is added
static PLYProperty g_ply_property;
%}

%%

^"comment "[^\n]*$ { /* pass over */ }
    /* should have <Magic>, --- how to make this the start state? */
^"ply"$ { BEGIN Versioning; debug("read magic number"); }
<Versioning>^"format ascii 1.0"$                { BEGIN Elements; g_ply.format = PLY_FORMAT_ASCII_1; debug("version read"); }
<Versioning>^"format binary_little_endian 1.0"$ { BEGIN Elements; g_ply.format = PLY_FORMAT_BINARY_LITTLE_ENDIAN_1; debug("version read"); }
<Versioning>^"format binary_big_endian 1.0"$    { BEGIN Elements; g_ply.format = PLY_FORMAT_BINARY_BIG_ENDIAN_1; debug("version read"); }
    /* remember to alter duplicate for properties state as well. need to remove this duplication.
        could have Properties go to the Elements state and back up a character when getting an unknown input, so Elements can handle it (e.g. get end_header). */
<Elements>^"element " { g_at_least_one_element = true; BEGIN ElementName; debug("element started reading"); }
<ElementName>[A-z]+ {
    BEGIN ElementCount;
    g_ply_element.name = (char *) malloc((strlen(yytext) + 1) * sizeof(char));
    mem_check(g_ply_element.name);
    strncpy(g_ply_element.name, yytext, strlen(yytext));
    debug("read element name");
}
<ElementCount>" "[0-9]+$ {
    BEGIN Properties;
    g_ply_element.count = atoi(yytext);
    add_element();
    debug("read element count, finished reading new element declaration");
}
<Properties>^"property list "    { BEGIN PropertyListCountType; g_ply_property.is_list = true; debug("property list started reading"); }
<PropertyListCountType>"uint "   { BEGIN PropertyType; g_ply_property.list_count_type = PLY_UINT; debug("property list count type read"); }
<PropertyListCountType>"uchar "  { BEGIN PropertyType; g_ply_property.list_count_type = PLY_UCHAR; debug("property list count type read"); }
<PropertyListCountType>"ushort " { BEGIN PropertyType; g_ply_property.list_count_type = PLY_USHORT; debug("property list count type read"); }
<Properties>^"property " {
    BEGIN PropertyType;
    g_ply_property.is_list = false;
    g_ply_property.list_count_type = PLY_NONE_TYPE;
    debug("property started reading");
}
<PropertyType>"char "     { BEGIN PropertyName; g_ply_property.type = PLY_CHAR; debug("property type read"); }
<PropertyType>"uchar "    { BEGIN PropertyName; g_ply_property.type = PLY_UCHAR; debug("property type read"); }
<PropertyType>"short "    { BEGIN PropertyName; g_ply_property.type = PLY_SHORT; debug("property type read"); }
<PropertyType>"ushort "   { BEGIN PropertyName; g_ply_property.type = PLY_USHORT; debug("property type read"); }
<PropertyType>"int "      { BEGIN PropertyName; g_ply_property.type = PLY_INT; debug("property type read"); }
<PropertyType>"uint "     { BEGIN PropertyName; g_ply_property.type = PLY_UINT; debug("property type read"); }
<PropertyType>"float "    { BEGIN PropertyName; g_ply_property.type = PLY_FLOAT; debug("property type read"); }
<PropertyType>"double "   { BEGIN PropertyName; g_ply_property.type = PLY_DOUBLE; debug("property type read"); }
<PropertyName>[A-z]+ {
    BEGIN Properties;
    g_ply_property.name = (char *) malloc((strlen(yytext) + 1) * sizeof(char));
    mem_check(g_ply_property.name);
    strncpy(g_ply_property.name, yytext, strlen(yytext));
    add_property();
    debug("property name read, finished reading new property declaration");
}
    /* duplicate rule ... would like to do something like <Element,Properties> */
<Properties>^"element " { g_at_least_one_element = true; BEGIN ElementName; debug("element started reading"); }
<Properties>^"end_header"$ {
    if (!g_at_least_one_element) _lex_error("no elements defined");
    return END_HEADER;
}
"\n" { /* */ }
"." { return ERROR; }

%%
#include <stdbool.h>
#include "helper_definitions.h"
#include "ply.h"


static void add_element(void)
{
    PLYElement *new_element = (PLYElement *) malloc(sizeof(PLYElement));
    mem_check(new_element);
    memcpy(new_element, &g_ply_element, sizeof(PLYElement));

    if (g_ply.first_element == NULL) {
        g_ply.first_element = new_element;
    } else {
        PLYElement *last_element = g_ply.first_element;
        while (last_element->next_element != NULL) last_element = last_element->next_element;
        last_element->next_element = new_element;
    }
    g_last_ply_element = new_element; // available adding new properties
    init_ply_element(&g_ply_element);
    g_ply.num_elements ++;
}

static void add_property(void)
{
    PLYProperty *new_property = (PLYProperty *) malloc(sizeof(PLYProperty));
    mem_check(new_property);
    memcpy(new_property, &g_ply_property, sizeof(PLYProperty));

    if (g_last_ply_element->first_property == NULL) {
        g_last_ply_element->first_property = new_property;
    } else {
        PLYProperty *last_property = g_last_ply_element->first_property;
        while (last_property->next_property != NULL) last_property = last_property->next_property;
        last_property->next_property = new_property;
    }
    init_ply_property(&g_ply_property);
    g_last_ply_element->num_properties ++;
}


static void debug(char *debug_string)
{
    if (g_debugging) puts(debug_string);
}


static void _lex_error(char *string)
{
    fprintf(stderr, ERROR_ALERT "Lexing error: %s\n", string);
    exit(EXIT_FAILURE);
}

PLY *read_ply(char *filename)
{
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, ERROR_ALERT "Couldn't open PLY file for lexing");
        exit(EXIT_FAILURE);
    }

    PLY *ply = (PLY *) malloc(sizeof(PLY));
    mem_check(ply);

    yyin = file;
    init_ply(&g_ply);
    init_ply_element(&g_ply_element);
    init_ply_property(&g_ply_property);
    if (yylex() == ERROR) _lex_error("ERROR");
    memcpy(ply, &g_ply, sizeof(PLY));

    ply->filename = (char *) malloc((strlen(filename) + 1) * sizeof(char));
    mem_check(ply->filename);
    strncpy(ply->filename, filename, strlen(filename));

    fclose(file);
    return ply;
}
