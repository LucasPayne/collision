/*================================================================================
    flex+C module for creating an internal PLY handle from a file.

    Hopefully, with flex it is easier to make sure the lexing really matches common uses/
    standards of PLY files. The way it is done here is with a finite state machine,
    building up a global PLY handle with elements and properties. Each state tries
    to match certain regular expressions/strings to build up information for the PLY handle.

notes:
================================================================================*/
%option noyywrap nodefault
%option prefix="_PLY_HEADER_yy"
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

const bool g_debugging = false;

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
<Elements,Properties>^"element " {
    // A new element has started declaration.
    g_at_least_one_element = true;
    BEGIN ElementName;
    debug("element started reading"); }
<ElementName>[A-z]+ {
    // Allocate memory for element name.
    BEGIN ElementCount;
    g_ply_element.name = (char *) malloc((strlen(yytext) + 1) * sizeof(char));
    mem_check(g_ply_element.name);
    strcpy(g_ply_element.name, yytext);
    debug("read element name");
}
<ElementCount>" "[0-9]+$ {
    // The new element has completed declaration, add this.
    BEGIN Properties;
    g_ply_element.count = atoi(yytext);
    add_element();
    debug("read element count, finished reading new element declaration");
}
<Properties>^"property list "    {
    // A new list-type property has started declaration.
    BEGIN PropertyListCountType;
    g_ply_property.is_list = true;
    debug("property list started reading");
}
    /* Read the unsigned integer type of the list count. */
<PropertyListCountType>"uint "   { BEGIN PropertyType; g_ply_property.list_count_type = PLY_UINT; debug("property list count type read"); }
<PropertyListCountType>"uchar "  { BEGIN PropertyType; g_ply_property.list_count_type = PLY_UCHAR; debug("property list count type read"); }
<PropertyListCountType>"ushort " { BEGIN PropertyType; g_ply_property.list_count_type = PLY_USHORT; debug("property list count type read"); }
<Properties>^"property " {
    // A new regular property has started declaration.
    BEGIN PropertyType;
    g_ply_property.is_list = false;
    g_ply_property.list_count_type = PLY_NONE_TYPE;
    debug("property started reading");
}
    /* Read the type of the property. */
<PropertyType>"char "     { BEGIN PropertyName; g_ply_property.type = PLY_CHAR; debug("property type read"); }
<PropertyType>"uchar "    { BEGIN PropertyName; g_ply_property.type = PLY_UCHAR; debug("property type read"); }
<PropertyType>"short "    { BEGIN PropertyName; g_ply_property.type = PLY_SHORT; debug("property type read"); }
<PropertyType>"ushort "   { BEGIN PropertyName; g_ply_property.type = PLY_USHORT; debug("property type read"); }
<PropertyType>"int "      { BEGIN PropertyName; g_ply_property.type = PLY_INT; debug("property type read"); }
<PropertyType>"uint "     { BEGIN PropertyName; g_ply_property.type = PLY_UINT; debug("property type read"); }
<PropertyType>"float "    { BEGIN PropertyName; g_ply_property.type = PLY_FLOAT; debug("property type read"); }
<PropertyType>"double "   { BEGIN PropertyName; g_ply_property.type = PLY_DOUBLE; debug("property type read"); }
<PropertyName>[A-z]+ {
    // The new property has completed declaration, add this.
    BEGIN Properties;
    g_ply_property.name = (char *) malloc((strlen(yytext) + 1) * sizeof(char));
    mem_check(g_ply_property.name);
    strcpy(g_ply_property.name, yytext);
    add_property();
    debug("property name read, finished reading new property declaration");
}
<Properties>^"end_header"$ {
    // The header is complete. Make sure at least one element has been declared.
    if (!g_at_least_one_element) _lex_error("no elements defined");
    return END_HEADER;
}
    /* States are inclusive, so these actions eat newlines and catch errors. */
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
        fprintf(stderr, ERROR_ALERT "Couldn't open PLY file for lexing.");
        exit(EXIT_FAILURE);
    }
    BEGIN INITIAL;
    yyrestart(file);

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
    strcpy(ply->filename, filename);

    fclose(file);
    return ply;
}
