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
#include "dict.h"

static char *DictNode_key(DictNode *node);
static char *DictNode_value(DictNode *node);
static uint32_t make_hash(char *key);

#define g_buf_size 1024
static char g_key[g_buf_size];
static Dictionary *g_dictionary;

int g_comment_depth = 0; // can't do this in C, but be fine with nesting /* */ comments.

%}

%%

    /* "thing:%s,stuff:%d,nice:%d" */


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
<Key,CommentOneLine>"/*" { printf("GOOOO "); g_comment_depth = 1; BEGIN Comment; }
<Key>"//" { printf("magic missile --> "); BEGIN CommentOneLine; }
<CommentOneLine>. { printf("ah "); }
<CommentOneLine>"\n" { printf(" out of mana\n"); BEGIN Key; }
<Key>[ \n\t] { /* */ }
<Key>. {
    fprintf(stderr, "Failed reading key.\n");
    exit(EXIT_FAILURE);
}
<Comment>"/*" { printf("HERE WE GO AGAIN "); g_comment_depth ++; }
<Comment>"*/" { printf(" brrrsshhh\n"); g_comment_depth --; if (g_comment_depth == 0) BEGIN Key; }
<Comment>.|"\n" { printf("vroom-vroom "); /* */ }

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
    fprintf(stderr, "Failed reading value.\n");
    exit(EXIT_FAILURE);
}

%%
#include <stdio.h>
#include <stdbool.h>

static char *DictNode_key(DictNode *node)
{
    return &(node->_string);
}
static char *DictNode_value(DictNode *node)
{
    return &(node->_string) + node->value_pos;
}


static uint32_t make_hash(char *key)
{
    char *c = key;
    uint32_t hash = 0, i = 0;
    /* while (*c++ != '\0' && ++i) hash += (i * *c) % (2 * *c + 256 * i); */
    /* while (*c++ != '\0' && ++i) hash += (i * i * i * i * *c) % (2 * *c + 256 * i); */
    int p = 119569;
    while (*c++ != '\0' && ++i) { int a=1; for (int j=0;j<i-1;j++){a*=123;}hash+=a* *c + i*i*i*i;}
    return hash%p;
}

void dict_add(Dictionary *dictionary, char *key, char *value)
{
    uint32_t hash = make_hash(key) % dictionary->size;

    DictNode *cur = dictionary->table[hash];
    DictNode *last = cur;
    while (cur != NULL) {
        last = cur;
        cur = cur->next;
    }
    DictNode *new = calloc(1, sizeof(DictNode) + strlen(key) + 0 + strlen(value) + 1); // The char entry accounted for in sizeof gives room for an extra null-terminator.
    if (new == NULL) {
        fprintf(stderr, "ERROR: Could not allocate memory for new entry in dictionary.\n");
        exit(EXIT_FAILURE);
    }
    new->next = NULL;
    strcpy(&(new->_string), key);
    new->value_pos = strlen(key) + 1;
    strcpy(&(new->_string) + new->value_pos, value);
    if (last == NULL) {
        dictionary->table[hash] = new;
    } else {
        ((DictNode *) last)->next = new;
    }
}
bool dict_get(Dictionary *dictionary, char *key, char *buffer, size_t buffer_size)
{
    uint32_t hash = make_hash(key) % dictionary->size;
    DictNode *cur = dictionary->table[hash];
    while (cur != NULL) {
        if (strcmp(DictNode_key(cur), key) == 0) {
            if (strlen(DictNode_value(cur)) > buffer_size - 1) return false;
            strncpy(buffer, DictNode_value(cur), buffer_size);
            return true;
        }
        cur = cur->next;
    }
    return false;
}

void ___free_end_dict_list(DictNode *cur)
{
    if (cur->next != NULL) ___free_end_dict_list(cur->next);
    free(cur);
}
void destroy_dictionary(Dictionary *dictionary)
{
    if (dictionary != NULL) {
        if (dictionary->table != NULL) {
            for (int i = 0; i < dictionary->size; i++) {
                if (dictionary->table[i] != NULL) {
                    ___free_end_dict_list(dictionary->table[i]);
                }
            }
            free(dictionary->table);
        }
        free(dictionary);
    }
}
Dictionary *new_dictionary(uint32_t size)
{
    Dictionary *dic = (Dictionary *) calloc(1, sizeof(Dictionary));
    if (dic == NULL) goto mem_error;
    dic->size = size;
    dic->table = (DictNode **) calloc(size, sizeof(DictNode *));
    if (dic->table == NULL) goto mem_error;
    return dic;
mem_error:
    fprintf(stderr, "ERROR: Failed to allocate memory for dictionary.\n");
    exit(EXIT_FAILURE);
}

void print_dictionary(Dictionary *dictionary)
{
    printf("DICTIONARY\n");
    printf("SIZE: %u\n", dictionary->size);
    for (int i = 0; i < dictionary->size; i++) {
        printf("[");
        DictNode *cur = dictionary->table[i];
        int num = 0;
        while(cur != NULL && ++num) {
            printf("%s[%s]: %s", num==1?"":", ",
                                 DictNode_key(cur),
                                 DictNode_value(cur));
            cur = cur->next;
        }
        printf("]\n");
    }
}
//--------------------------------------------------------------------------------
// Testing
//--------------------------------------------------------------------------------
void test_get(char *key)
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
