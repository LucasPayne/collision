%option noyywrap nodefault
%x PROJECT_LIBS
%x APPLICATION_LIBS

LEAD    [\*\s]*

%{
#include <ctype.h>

static void print_object_file(char *name, char *path);
static char *g_app_filename;
%}

%%
^[\*\s]*"PROJECT_LIBS:"\s*$ {
    BEGIN PROJECT_LIBS;
}
<PROJECT_LIBS>{LEAD}"-" {
    /* */
    int c;
    while ((c = input()) && c != '\n');
}
<PROJECT_LIBS>{LEAD}"+" { print_object_file(yytext, "build/lib"); }
<APPLICATION_LIBS>{LEAD}"+" { char path[512]; /* ... */ sprintf(path, "src/%s/build", g_app_filename); print_object_file(yytext, path); }
<PROJECT_LIBS>[ \n\t]* {
    /* */
}
<PROJECT_LIBS>. {
    BEGIN INITIAL;
}
. { /* */ }
"\n" { /* */ }

%%

static void print_object_file(char *name, char *path)
{
    puts(path);
    putchar('/');
    int c;
    while ((c = input()) && c != '\n') { 
        if (!isspace(c)) putchar(c);
    }
    printf(".o ");
}


int main(int argc, char *argv[])
{
// takes input from stdin
    if (argc != 2) exit(EXIT_FAILURE);
    char *g_app_filename = argv[1]; // does this work? yylex is only called here. is this bad?

    yyin = stdin;
    yylex();
}
