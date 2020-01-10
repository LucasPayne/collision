%option noyywrap nodefault
%option prefix="dd_yy"

%{
#include "data_dictionary_implementation.h"
#include "tokens.yy.h"
#include <stdbool.h>

// The parser is generated with bison flag "-p ...", redefining so definitions the lexer uses from the bison's generated header file.
#define yylval dd_yylval

#define trace_lex 0

#define watch_lex 0
#if watch_lex == 1
#define RETURN(TOKEN)\
{\
    puts(#TOKEN);\
    getchar();\
    return ( TOKEN );\
}
#else
#define RETURN(TOKEN)\
{\
    return ( TOKEN );\
}
#endif
// Lexing
// The file stack, for #include(...) directives.
// flex and bison, page 28
struct FileStackEntry {
    FILE *file;
    struct FileStackEntry *prev;
    YY_BUFFER_STATE buffer_state; //page 26 for details on YY_BUFFER_STATE and flex's
                                  //defaults for getting a stream to lex from, with yyin.
};
struct FileStackEntry *g_file_stack_top = NULL;

%}
%x ValueText
%x CommentOneLine
%x Directive

NAME    [/_A-Za-z0-9]+

%%

"//" { BEGIN CommentOneLine; }
<CommentOneLine>. { /* */ }
<CommentOneLine>"\n" { BEGIN INITIAL; }

"#" {
    if (trace_lex) printf("Reading #-directive...\n");
    BEGIN Directive;
}
<Directive>"include("{NAME}");" {
    char *start = strchr(yytext, '(') + 1;
    char *end = strchr(yytext, ')');
    const int buf_size = 4096;
    if (end - start >= buf_size - 3) {
        fprintf(stderr, "ERROR: Name in #include(...) is too long.\n");
        exit(EXIT_FAILURE);
    }
    char buf[buf_size];
    strncpy(buf, start, end - start);
    strcpy(buf + (end - start), ".dd");
    buf[(end - start) + 3] = '\0';
    FILE *file = fopen(buf, "r");
    if (file == NULL) {
        fprintf(stderr, "Attempted to include unknown data-definition file \"%s\".\n", buf);
        exit(EXIT_FAILURE);
    }
    dd_push_file(file);
    if (trace_lex) printf("Started including data-definition file \"%s\".\n", buf);
    BEGIN INITIAL;
}

{NAME} {
    if (trace_lex) printf("IDENTIFIER: %s\n", yytext);
    yylval.symbol = new_symbol(yytext); RETURN(IDENTIFIER);
}
";" |
"<" |
"(" |
")" { if (trace_lex) printf("symbol: %c\n", yytext[0]); RETURN(yytext[0]); }

":" {
    if (trace_lex) printf("symbol: :\n");
    if (trace_lex) printf("getting value text\n");
    BEGIN ValueText;
    RETURN(':');
}
" "|"\t"|"\n" { /* eat whitespace */ }

<<EOF>> {
    // Complete lexing if at the bottom of the file stack, or pop a file and continue lexing where the previous open file was left of.
    dd_pop_file();
    if (g_file_stack_top == NULL) {
        if (trace_lex) printf("Lexing completed.\n");
        yyterminate();
    }
}

<ValueText>" "[^;]* {
    if (trace_lex) printf("VALUE_TEXT: \"%s\":\n", yytext);
    yylval.symbol = new_symbol(yytext + 1);
    BEGIN INITIAL;
    RETURN(VALUE_TEXT);
}

%%

// #include(...) directives.
void dd_push_file(FILE *file)
{
    struct FileStackEntry *new = (struct FileStackEntry *) malloc(sizeof(struct FileStackEntry));
    mem_check(new);
    new->file = file;
    new->prev = g_file_stack_top;
    new->buffer_state = yy_create_buffer(new->file, YY_BUF_SIZE);
    yy_switch_to_buffer(new->buffer_state);
    g_file_stack_top = new;
}
void dd_pop_file(void)
{
    if (g_file_stack_top == NULL) {
        fprintf(stderr, "ERROR: Attempted to pop from empty file-stack.\n");
        exit(EXIT_FAILURE);
    }
    struct FileStackEntry *to_free = g_file_stack_top;
    g_file_stack_top = g_file_stack_top->prev;
    fclose(to_free->file);
    yy_delete_buffer(to_free->buffer_state);
    free(to_free);

    // If the stack is still non-empty, then switch to the previous buffer.
    if (g_file_stack_top != NULL) yy_switch_to_buffer(g_file_stack_top->buffer_state);
}
