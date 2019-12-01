/*
    Testing lex linking
*/
%option noyywrap nodefault

%%

[A-z]+ { puts(yytext); puts(yytext); }
. { printf("wow"); }

%%

void test_the_lex(void)
{
    printf("the lex is connected and is going\n");
}
