/*================================================================================
  This header contains the non-interface declarations for the data-dictionary
  module.
================================================================================*/
#ifndef HEADER_DEFINED_DATA_DICTIONARY_IMPLEMENTATION
#define HEADER_DEFINED_DATA_DICTIONARY_IMPLEMENTATION
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include "data_dictionary.h" // include the module-user interface.

// Implemented in lexer file.
extern void dd_push_file(FILE *file);
extern void dd_pop_file(void);

DataDictionary *new_data_dictionary(void);
DataDictionary *resolve_dictionary_expression(DataDictionary *dict, DictExpression *expression);
uint32_t hash_crc32(char *string);
bool mask_dictionary_to_table(DataDictionary *dict_table, EntryNode *dict);

DictExpression *lookup_dict_expression(DataDictionary *dict, char *path, DataDictionary **new_parent_dict);
DictExpression *scoped_dictionary_expression(DataDictionary *dict, char *name, DataDictionary **new_parent_dict);

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

extern FILE *dd_yyin;

//--- Why do these need to be declared here?
int dd_yylex(void);
// Looks like you need to implement this yourself unless there is something like "yynoerror".
void dd_yyerror(char *errmsg);

#endif // HEADER_DEFINED_DATA_DICTIONARY_IMPLEMENTATION
