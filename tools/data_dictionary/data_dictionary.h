#ifndef HEADER_DEFINED_DATA_DICTIONARY
#define HEADER_DEFINED_DATA_DICTIONARY
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

// mem_check defined here before integrated into regular modules.
#define mem_check(POINTER)\
    if (( POINTER ) == NULL) {\
        fprintf(stderr, "malloc failed.\n");\
        exit(EXIT_FAILURE);\
    }

// Implemented in lexer file.
extern void push_file(FILE *file);
extern void pop_file(void);

// AST
struct DictExpression_s;
struct EntryNode_s;
typedef struct DictExpression_s {
    struct DictExpression_s *next;
    bool is_name;
    // Kind: name
    int name; //symbol
    // Kind: dict-literal
    struct EntryNode_s *dict;
} DictExpression;
typedef struct EntryNode_s {
    struct EntryNode_s *next;
    bool is_dict;
    int name; //symbol
    // Kind: entry
    int type; //symbol, -1 as no type.
    int value_text; // symbol, -1 as no value text.
    // Kind: dict
    struct DictExpression_s *dict_expression;
} EntryNode;

// Dictionary tables (hash table where dictionaries are masked together)
typedef struct DictionaryTableCell_s {
    int name; // -1: empty cell.
    union {
        struct {
            int32_t type;
            int32_t value_text;
        } value;
        struct {
            DictExpression *dict_expression;
        } dict;
    } contents;
    bool is_dict;
} DictionaryTableCell;

typedef struct Dictionary_s {
    // Be careful when allocating memory for this "variable-size struct".
    int table_size;
    struct Dictionary_s *parent_dictionary; // for scoping.
    DictionaryTableCell *table;
    DictionaryTableCell ___table;
} Dictionary;

Dictionary *new_dictionary(void);
Dictionary *resolve_dictionary_expression(Dictionary *dict, DictExpression *expression);
uint32_t crc32(char *string);
bool mask_dictionary_to_table(Dictionary *dict_table, EntryNode *dict);

// Lookup in dictionary tables.
DictExpression *lookup_dict_expression(Dictionary *dict, char *name);
Dictionary *lookup_dict(Dictionary *dict, char *name);
bool lookup_value(Dictionary *dict, char *name);

DictExpression *scoped_dictionary_expression(Dictionary *dict, char *name);



EntryNode *new_entry_node(int name_symbol, int type_symbol, int value_text_symbol);
EntryNode *new_dict_node(int name_symbol, DictExpression *dict_expression);
DictExpression *new_named_dict_expression(int name_symbol);
DictExpression *new_literal_dict_expression(EntryNode *dict);
extern EntryNode *g_dict; // for accessing the root of the AST after calling yyparse.

void print_ast(EntryNode *dict);
void print_dict_expression(DictExpression *expression);

// symbol table
int new_symbol(char *string);
void print_symbol_table(void);
char *symbol(int entry);

extern FILE *yyin;

//--- Why do these need to be declared here?
int yylex(void);
// Looks like you need to implement this yourself unless there is something like "yynoerror".
void yyerror(char *errmsg);

#endif // HEADER_DEFINED_DATA_DICTIONARY
