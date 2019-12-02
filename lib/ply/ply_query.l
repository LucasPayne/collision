/*================================================================================
    flex+C module for interpreting PLY queries.
================================================================================*/
    /*================================================================================
        Upper C declarations and global variable definitions.
    ================================================================================*/
%option noyywrap nodefault
%option prefix="_PLY_QUERY_yy"
%x ElementPatterns
%x Properties
%x PropertyPattern

%{
#include "helper_definitions.h"
#include "ply.h"

#define ERROR 0
#define DONE 1

static const bool g_debugging = true;
static void debug(char *debug_string);

static void lex_error(char *error_string);
static void add_query_element(void);
static void add_query_property(void);

static PLYQuery g_ply_query;
static PLYQueryElement g_ply_query_element;
static PLYQueryElement *g_last_ply_query_element;
static PLYQueryProperty g_ply_query_property;
%}

%%
    /*================================================================================
        Patterns and actions
    ================================================================================*/
<ElementPatterns>"["[A-z]+("|"[A-z]+)*"]" {
    int c;
    // Must be followed by ": "
    if ((c = input()) != ':' || (c = input()) != ' ') {
        lex_error("error");
        return ERROR;
    }
    g_ply_query_element.pattern_string = (char *) malloc((strlen(yytext) + 1 - 2) * sizeof(char)); // -2 since the brackets are removed
    mem_check(g_ply_query_element.pattern_string);
    strncpy(g_ply_query_element.pattern_string, yytext + 1, strlen(yytext) - 2); // ---check if this is correct
    add_query_element();
    debug("New element pattern declaration started");
    BEGIN Properties;
}
<ElementPatterns>"\n" { /* eat newlines */ }
<ElementPatterns>. { lex_error("bad element pattern"); return ERROR; }
<Properties>"list float " { g_ply_query_property.is_list = true; g_ply_query_property.pack_type = PLY_FLOAT; BEGIN PropertyPattern; debug("property declaration started, property type given"); }
<Properties>"list int " { g_ply_query_property.is_list = true; g_ply_query_property.pack_type = PLY_INT; BEGIN PropertyPattern; debug("property declaration started, property type given"); }
<Properties>"float " { g_ply_query_property.is_list = false; g_ply_query_property.pack_type = PLY_FLOAT; BEGIN PropertyPattern; debug("property declaration started, property type given"); }
<Properties>"int " { g_ply_query_property.is_list = false; g_ply_query_property.pack_type = PLY_INT; BEGIN PropertyPattern; debug("property declaration started, property type given"); }
<Properties>"\n" { /* eat newlines */ }
<Properties>. { lex_error("bad property"); return ERROR; }
<PropertyPattern>[A-z]+("|"[A-z]+)* {
    int c;
    // query string must finish or this pattern must be followed by ", "
    bool finished = ((c = input()) == EOF || c == '\0');
    if (!finished) {
        if (c != ',' || (c = input()) != ' ') {
            lex_error("error");
        }
    }
    g_ply_query_property.pattern_string = (char *) malloc((strlen(yytext) + 1) * sizeof(char));
    mem_check(g_ply_query_property.pattern_string);
    strcpy(g_ply_query_property.pattern_string, yytext);
    add_query_property();

    debug("property declaration complete");
    if (finished) return DONE;
    BEGIN Properties;
}
    /* <PropertyPattern>"[" { */
    /*     // Go back to reading element patterns */
    /*     unput(); */
    /*     BEGIN ElementPatterns; */
    /* } */
<PropertyPattern>"\n" { /* eat newlines */ }
<PropertyPattern><<EOF>> { return DONE; }
<PropertyPattern>. { lex_error("bad property pattern"); return ERROR; }

%%
    /*================================================================================
        Main C code.
    ================================================================================*/
#include "helper_definitions.h"
#include "ply.h"

static void lex_error(char *error_string)
{
    fprintf(stderr, ERROR_ALERT "Lexing error: %s\n", error_string);
    exit(EXIT_FAILURE);
}
static void debug(char *debug_string)
{
    if (g_debugging) puts(debug_string);
}
static void add_query_element(void)
{
    PLYQueryElement *new_query_element = (PLYQueryElement *) malloc(sizeof(PLYQueryElement));
    mem_check(new_query_element);
    memcpy(new_query_element, &g_ply_query_element, sizeof(PLYQueryElement));

    if (g_ply_query.first_element == NULL) {
        g_ply_query.first_element = new_query_element;
    } else {
        PLYQueryElement *last_element = g_ply_query.first_element;
        while (last_element->next != NULL) last_element = last_element->next;
        last_element->next = new_query_element;
    }
    g_last_ply_query_element = new_query_element;
    memset(&g_ply_query_element, 0, sizeof(PLYQueryElement));
    g_ply_query.num_elements ++;
}
static void add_query_property(void)
{
    PLYQueryProperty *new_query_property = (PLYQueryProperty *) malloc(sizeof(PLYQueryProperty));
    mem_check(new_query_property);
    memcpy(new_query_property, &g_ply_query_property, sizeof(PLYQueryProperty));

    if (g_last_ply_query_element->first_property == NULL) {
        g_last_ply_query_element->first_property = new_query_property;
    } else {
        PLYQueryProperty *last_property = g_last_ply_query_element->first_property;
        while (last_property->next != NULL) last_property = last_property->next;
        last_property->next = new_query_property;
    }
    memset(&g_ply_query_property, 0, sizeof(PLYQueryProperty));
    g_last_ply_query_element->num_properties ++;
}

PLYQuery *read_ply_query(char *query_string)
{
    printf("Reading ply query ...\n");

    memset(&g_ply_query, 0, sizeof(PLYQuery));
    memset(&g_ply_query_element, 0, sizeof(PLYQueryElement));
    memset(&g_ply_query_property, 0, sizeof(PLYQueryProperty));
    g_last_ply_query_element = NULL;

    printf("Starting lexer ...\n");
    /* for (int i = 0; i < 500000000; i++) continue; */

    printf("query string: %s\n", query_string);
    YY_BUFFER_STATE yy_buffer = yy_scan_bytes(query_string, strlen(query_string) + 1);
    yy_switch_to_buffer(yy_buffer);
    BEGIN ElementPatterns;

    /* for (int i = 0; i < 500000000; i++) continue; */

    if (yylex() == ERROR) return NULL;
    printf("lexer complete\n");

    PLYQuery *query = (PLYQuery *) malloc(sizeof(PLYQuery));
    mem_check(query);
    memcpy(query, &g_ply_query, sizeof(PLYQuery));
    query->query_string = (char *) malloc((strlen(query_string) + 1) * sizeof(char));
    mem_check(query->query_string);
    strcpy(query->query_string, query_string);

    yy_delete_buffer(yy_buffer);
    return query;
}
