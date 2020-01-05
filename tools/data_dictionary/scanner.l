%option noyywrap nodefault
%{
#include "data_dictionary.h"
#include "tokens.yy.h"
#include <stdbool.h>

#define trace_lex 0
%}
%x ValueText

%%

[/_A-Za-z0-9]+ {
    if (trace_lex) printf("IDENTIFIER: %s\n", yytext);
    yylval.symbol = new_symbol(yytext); return IDENTIFIER;
}
";" |
"<" |
"(" |
")" { if (trace_lex) printf("symbol: %c\n", yytext[0]); return yytext[0]; }

":" {
    if (trace_lex) printf("symbol: :\n");
    if (trace_lex) printf("getting value text\n");
    BEGIN ValueText;
    return ':';
}
" "|"\t"|"\n" { /* eat whitespace */ }

<ValueText>" "[^;]* {
    if (trace_lex) printf("VALUE_TEXT: \"%s\":\n", yytext);
    yylval.symbol = new_symbol(yytext + 1);
    BEGIN INITIAL;
    return VALUE_TEXT;
}

%%
