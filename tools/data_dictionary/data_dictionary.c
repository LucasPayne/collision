#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "data_dictionary.h"

//- Symbol table -----------------------------------------------------------------
// Taken from the gen_shader_blocks code.
static char *symbol_table = NULL;
static size_t symbol_table_size;
static int symbol_table_position = 0;
#define SYMBOL_TABLE_START_SIZE 3000

int new_symbol(char *string)
{
    #define table_check() if (symbol_table == NULL) { fprintf(stderr, "ERROR: Could not allocate memory for symbol table.\n"); exit(EXIT_FAILURE); }
    if (symbol_table == NULL) {
        symbol_table_size = SYMBOL_TABLE_START_SIZE;
        symbol_table_position = 0;
        symbol_table = (char *) calloc(1, symbol_table_size * sizeof(char));
        table_check();
    } else if (symbol_table_position + strlen(string) >= symbol_table_size) {
        symbol_table_size *= 2;
        symbol_table = (char *) realloc(symbol_table, symbol_table_size * sizeof(char));
        table_check();
    }
    int entry = symbol_table_position;
    strcpy(symbol_table + symbol_table_position, string);
    symbol_table_position += strlen(string) + 1;
    /* print_symbol_table(); */
    return entry;
}
void print_symbol_table(void)
{
    printf("------------\n");
    printf("SYMBOL TABLE\n");
    printf("------------\n");
    for (int i = 0; i < symbol_table_position; i++) {
        if (symbol_table[i] == '\0') putchar('\n');
        else putchar(symbol_table[i]);
    }
    printf("------------\n");
}
char *symbol(int entry)
{
    return symbol_table + entry;
}
//--------------------------------------------------------------------------------
// AST
#define ast_mem_check(THING)\
    if (( THING ) == NULL) {\
        fprintf(stderr, "ERROR: Could not allocate memory for a new node when building AST.\n");\
        exit(EXIT_FAILURE);\
    }
EntryNode *new_entry_node(int name_symbol, int type_symbol, int value_text_symbol)
{
    EntryNode *node = (EntryNode *) calloc(1, sizeof(EntryNode));
    ast_mem_check(node);
    node->is_dict = false;
    node->name = name_symbol;
    node->type = type_symbol;
    node->value_text = value_text_symbol;
    return node;
}
EntryNode *new_dict_node(int name_symbol, DictExpression *dict_expression)
{
    EntryNode *node = (EntryNode *) calloc(1, sizeof(EntryNode));
    ast_mem_check(node);
    node->is_dict = true;
    node->name = name_symbol;
    node->dict_expression = dict_expression;
    return node;
}

void yyerror(char *errmsg)
{
    fprintf(stderr, "Parsing error: %s\n", errmsg);
    exit(EXIT_FAILURE);
}

int main(void)
{
    yyin = fopen("tests/test1", "r");
    yyparse();
}




