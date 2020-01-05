#ifndef HEADER_DEFINED_DATA_DICTIONARY
#define HEADER_DEFINED_DATA_DICTIONARY
#include <stdbool.h>
#include <stdlib.h>

typedef struct DictExpression_s {

} DictExpression;
typedef struct EntryNode_s {
    struct EntryNode_s *next;
    bool is_dict;
    int name; //symbol
    // Kind: entry
    int type; //symbol, -1 as no type.
    int value_text; // symbol, -1 as no value text.
    // Kind: dict
    DictExpression *dict_expression;
} EntryNode;

EntryNode *new_entry_node(int name_symbol, int type_symbol, int value_text_symbol);
EntryNode *new_dict_node(int name_symbol, DictExpression *dict_expression);

int new_symbol(char *string);
void print_symbol_table(void);
char *symbol(int entry);

extern FILE *yyin; // will not be using files, but passing a string into the parser.

//--- Why do these need to be declared here?
int yylex(void);
// Looks like you need to implement this yourself unless there is something like "yynoerror".
void yyerror(char *errmsg);

#endif // HEADER_DEFINED_DATA_DICTIONARY
