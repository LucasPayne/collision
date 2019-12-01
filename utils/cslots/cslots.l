/*================================================================================

cslots utility
Intended for storing meta-information/build directives in a source file.

================================================================================*/
%option noyywrap nodefault
%x CSLOTS

LEAD    [\*\s]*

%{
#include <ctype.h>

static void print_entry(char *name);
static char g_leading_string[512];
static char g_pattern[512];
%}

%%
    /* maybe need to make this pattern more specific */
[A-z_0-9]+":"\s*$ {
    /* Checks this pattern against the leading string (slot group name) given as a command line argument.
     * Surely flex has some sort of thing to make this easier? Regexes must be compiled, but this more general one
     * can be further checked to change control flow. Just do it in C?
     */
    /* puts(yytext); */
    int i = 0;
    while (isspace(yytext[i]) || yytext[i] == '*') i++;
    int j = i;
    while (yytext[j] != ':') j++;
    yytext[j] = '\0';
    /* puts(yytext + i); */
    /* puts(g_leading_string); */
    if (strcmp(yytext + i, g_leading_string) == 0) {
        /* printf("beginning cslots\n"); */
        BEGIN CSLOTS;
    }
}
<CSLOTS>{LEAD}"-" {
    int c;
    while ((c = input()) && c != '\n');
}
<CSLOTS>{LEAD}"+" {
    print_entry(yytext);
}
<CSLOTS>[ \n\t]* {
    /* */
}
<CSLOTS>. {
    BEGIN INITIAL;
}
. { /* */ }
"\n" { /* */ }

%%

static void print_entry(char *name)
{
    char slot_name[512];
    int c;
    int i = 0;
    while ((c = input()) && c != '\n' && i < 512 - 1) { 
        if (!isspace(c)) slot_name[i++] = c;
    }
    slot_name[i] = '\0';
    for (int i = 0; g_pattern[i] != '\0'; i++) {
        if (strncmp(g_pattern + i, "{n}", 3) == 0) {
            // slot name print directive
            printf("%s", slot_name);
            i += 2;
	} else if (strncmp(g_pattern + i, "{h}", 3) == 0) {
            // head name print directive (the last in a directory path)
            char *p;
            if ((p = strrchr(slot_name, '/')) == NULL) {
                printf("%s", slot_name); // no path slashes
            } else {
                printf("%s", p + 1);
            }
            i += 2;
        } else {
            putchar(g_pattern[i]);
        }
    }
    putchar(' ');
}

static void arg_error(char *flag)
{
    fprintf(stderr, "bad flag: %s\n", flag);
    exit(EXIT_FAILURE);
}
static void print_help(void)
{
printf("usage:\n");
printf("\tcslots <slot group name> [--prefix <prefix>] [--suffix <suffix>]\n");
printf("\tPatterns:\n");
printf("\t\t>n : the name of the slot entry currently being printed\n");
printf("cslots:\n");
printf("\tFilter to pull meta-information/build directives out of source files.\n");
}

int main(int argc, char *argv[])
{
// takes input from stdin
    if (argc == 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        printf("HELP\n");
        print_help();
        return 0;
    }

    int got_input = 0;
    int got_pattern = 0;
    g_pattern[0] = '\0';
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--pattern") == 0) {
            if (got_pattern) arg_error("give only one pattern");
            got_pattern = 1;
            if (i + 1 >= argc) arg_error("-p, --pattern");
            strncpy(g_pattern, argv[i + 1], 512);
            i ++;
        }
        else if (got_input) {
            arg_error("invalid");
        } else {
            strncpy(g_leading_string, argv[i], 512);
            got_input = 1;
        }
    }
    if (!got_input) arg_error("no leading string given");
    if (!got_pattern) arg_error("no pattern given");

    yyin = stdin;
    yylex();
}
