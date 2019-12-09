%option noyywrap nodefault
%option prefix="_DICT_QUERY_yy"
%x Key
%x Pattern

%{
#define ERROR 0
#define SUCCESS 1
%}

%%

%%
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include "dictionary.h"

bool dict_query(Dictionary *dictionary, char *query_string, ...)
{
    va_list ap;
    va_start(ap, query_string);
    // Go through the argument list and prepare information
    // about how the query lexer should fill variables.
    // Or, I could just get the lexer to fill in a buffer of values, and return
    // ERROR or not, so the passed pointers aren't dereferenced unless it is correct.

    YY_BUFFER_STATE yy_buffer = yy_scan_bytes(query_string, strlen(query_string) + 1);
    yy_switch_to_buffer(yy_buffer);
    BEGIN Key;
    if (yylex() == ERROR) return false;
    yy_delete_buffer(yy_buffer);

    va_end(ap);
}
