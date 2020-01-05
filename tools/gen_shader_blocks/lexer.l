%option noyywrap nodefault
%{
#include "gen_shader_blocks.h"
#include "tokens.yy.h" //also contains the lvalue union.
%}
%x CommentOneLine

%%

"//" { BEGIN CommentOneLine; }
<CommentOneLine>. { /* */ }
<CommentOneLine>"\n" { BEGIN INITIAL; }

"#define "[_A-z0-9]+" "[0-9]+[\t ]*$ {
    // A #define integer definition is used as a single token, and its structure validated here in the lexer.
    // Then the token values are the symbol and the value.
    int identifier_start = strchr(yytext, ' ') - yytext + 1;
    int value_start = strchr(yytext + identifier_start + 1, ' ') - yytext + 1;
    int value;
    int num_read;
    char buf[1024];
    strncpy(buf, yytext, 1024);
    buf[value_start - 1] = '\0';
    sscanf(buf + value_start, "%d", &value);
    yylval.hash_definition.identifier = new_symbol(buf + identifier_start);
    yylval.hash_definition.value = value;
    return HASHDEFINE;
}
"block" { return BLOCK; }
"struct" { return STRUCT; }
"{" { return LEFTBRACE; }
"}" { return RIGHTBRACE; }
"[" { return LEFTBRACKET; }
"]" { return RIGHTBRACKET; }
";" { return SEMICOLON; }
[_A-Za-z0-9]+ { yylval.symbol = new_symbol(yytext); return IDENTIFIER; }
" "|"\t"|"\n" { /* eat whitespace */ }
<<EOF>> { return LEX_DONE; }
. { return LEX_ERROR; }

%%
