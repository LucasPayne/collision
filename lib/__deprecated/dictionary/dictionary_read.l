%option noyywrap nodefault
%option prefix="_DICT_READ_yy"
%x Key
%x Value
%x Comment
%x CommentOneLine

%{
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include "dictionary.h"

static char *DictNode_key(DictNode *node);
static char *DictNode_value(DictNode *node);
static uint32_t make_hash(char *key);

// Uncomment this for debugging.
/* #define DEBUG */
#ifdef DEBUG
#define debugp(STRING)\
{\
    printf("%s", ( STRING ));\
}
#undef DEBUG
#else
#define debugp(STRING)\
{\
}
#endif

#define g_buf_size 1024
static char g_key[g_buf_size];
static Dictionary *g_dictionary;

int g_comment_depth = 0; // can't do this in C, but be fine with nesting /* */ comments.

#define ERROR 111
#define SUCCESS 222
%}

%%

    /* Only succeed on file end if 0+ correct key-value pairs have been lexed. */
<Key><<EOF>> { return SUCCESS; }
<Key>[0-9A-z_]+":"[ \t\n]* {
    char *colon = strchr(yytext, ':');
    if (colon - yytext >= g_buf_size) {
        fprintf(stderr, "ERROR: Buffer size not large enough to store key.\n");
        exit(EXIT_FAILURE);
    }
    strncpy(g_key, yytext, colon - yytext);
    g_key[colon - yytext] = '\0';
    BEGIN Value;
}
<Key,CommentOneLine>"/*" { debugp("GOOOO "); g_comment_depth = 1; BEGIN Comment; }
<Key>"//" { debugp("magic missile --> "); BEGIN CommentOneLine; }
<CommentOneLine>. { debugp("ah "); }
<CommentOneLine>"\n" { debugp(" out of mana\n"); BEGIN Key; }
<Key>[ \n\t] { /* */ }
<Key>. {
    /* fprintf(stderr, "Failed reading key.\n"); */
    /* exit(EXIT_FAILURE); */
    return ERROR;
}
<Comment>"/*" { debugp("HERE WE GO AGAIN "); g_comment_depth ++; }
<Comment>"*/" { debugp(" brrrsshhh\n"); g_comment_depth --; if (g_comment_depth == 0) BEGIN Key; }
<Comment>.|"\n" { debugp("vroom-vroom "); /* */ }

<Value>[^\n]* {
    dict_add(g_dictionary, g_key, yytext);
#if 0
    // View the dictionary being filled.
    printf("Key: %s, Value: %s\n", g_key, yytext);
    printf("Hash: %u\n", make_hash(g_key) % g_dictionary->size);
    print_dictionary(g_dictionary);
    getchar();
#endif
    input(); // eat the newline
    BEGIN Key;
}
<Value>.|"\n" {
    /* fprintf(stderr, "Failed reading value.\n"); */
    /* exit(EXIT_FAILURE); */
    return ERROR;
}

%%
#include <stdio.h>
#include <stdbool.h>
#include "helper_definitions.h"
#include "dictionary.h"


Dictionary *dictionary_read(FILE *file)
{
    if (file == NULL) {
        fprintf(stderr, ERROR_ALERT "Attempted to read a dictionary with a null file handle.\n");
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_SET);
    yyrestart(file);
    yyin = file;

    g_dictionary = new_dictionary(50);
    BEGIN Key;
    if (yylex() == ERROR) {
        destroy_dictionary(g_dictionary);
        g_dictionary = NULL;
        return NULL;
    }
    Dictionary *got_dictionary = g_dictionary;
    g_dictionary = NULL;
    return got_dictionary;
}

#if 0
//--------------------------------------------------------------------------------
// Testing
//--------------------------------------------------------------------------------
static void test_get(char *key)
{
    const int buf_size = 1024;
    printf("Querying for \"%s\": ", key);
    char buf[buf_size];
    if (!dict_get(g_dictionary, key, buf, buf_size)) printf("Not found!\n");
    else printf("got \"%s\"\n", buf);
}
int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "give good args\n");
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        fprintf(stderr, "file failed to open\n");
        exit(EXIT_FAILURE);
    }
    yyin = file;

    destroy_dictionary(g_dictionary);
    g_dictionary = new_dictionary(50);


    BEGIN Key;
    yylex();

    print_dictionary(g_dictionary);

    test_get("thing");
    test_get("nice");
    test_get("ary");
    test_get("that");
}
#endif
