/*================================================================================
    flex+C module for interpreting PLY queries.
================================================================================*/
    /*================================================================================
        Upper C declarations and global variable definitions.
    ================================================================================*/
%option noyywrap nodefault
%x ElementPatterns
%x Properties
%x PropertyPattern

%{
#include "helper_definitions.h"
#include "ply.h"

#define ERROR 0
#define DONE 1
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

"" { BEGIN ElementPatterns; }

<ElementPatterns>"["[A-z]+("|"[A-z]+)*"]" {
    int c;
    // Must be followed by ": "
    if ((c = input()) != ':' || (c = input()) != ' ') {
        lex_error("error");
        return ERROR;
    }
    g_ply_query_element.pattern_string = (char *) malloc((strlen(yytext) + 1) * sizeof(char));
    mem_check(g_ply_query_element.pattern_string);
    strncpy(g_ply_query_element.pattern_string, yytext, strlen(yytext));
    add_query_element();
    BEGIN Properties;
}
<ElementPatterns>"\n" { /* eat newlines */ }
<ElementPatterns>. { lex_error("bad element pattern"); return ERROR; }
<Properties>"float32 " { g_ply_query_property.pack_type = PLY_FLOAT; BEGIN PropertyPattern; }
<Properties>"int32 " { g_ply_query_property.pack_type = PLY_INT; BEGIN PropertyPattern; }
<Properties>"\n" { /* eat newlines */ }
<Properties>. { lex_error("bad property"); return ERROR; }
<PropertyPattern>[A-z]+("|"[A-z]+)* {
    int c;
    // query string must finish or this pattern must be followed by ", "
    if ((c = input()) != EOF && (c != ',' || (c = input()) != ' ')) {
        lex_error("error");
    }
    g_ply_query_property.pattern_string = (char *) malloc((strlen(yytext) + 1) * sizeof(char));
    mem_check(g_ply_query_property.pattern_string);
    strncpy(g_ply_query_property.pattern_string, yytext, strlen(yytext));
    add_query_property();
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
    memset(&g_ply_query, 0, sizeof(PLYQuery));
    memset(&g_ply_query_element, 0, sizeof(PLYQueryElement));
    memset(&g_ply_query_property, 0, sizeof(PLYQueryProperty));
    yy_scan_string(query_string);

    if (yylex() == ERROR) return NULL;
    PLYQuery *query = (PLYQuery *) malloc(sizeof(PLYQuery));
    mem_check(query);
    memcpy(query, &g_ply_query, sizeof(PLYQuery));
    return query;
}
