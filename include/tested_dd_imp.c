/*--------------------------------------------------------------------------------
    bugs and problems:
        The IR is being changed when expressions are concatenated. If the IR form is still used,
        copy it when it is being appended to an expression.
--------------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "helper_definitions.h"
#include "data_dictionary_implementation.h"

//- Symbol table -----------------------------------------------------------------
// Taken from the gen_shader_blocks code.
static char *symbol_table = NULL;
static size_t symbol_table_size;
static int symbol_table_position = 0;
#define SYMBOL_TABLE_START_SIZE 3000

int new_symbol(char *string)
{
}
void print_symbol_table(void)
{
}
char *symbol(int entry)
{
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
}
EntryNode *new_dict_node(int name_symbol, DictExpression *dict_expression)
{
}

DictExpression *new_named_dict_expression(int name_symbol)
{
}
DictExpression *new_literal_dict_expression(EntryNode *dict)
{
}


static void _print_dict_expression(DictExpression *expression, int indent);
static void _print_ast(EntryNode *node, int indent);

static void _print_dict_expression(DictExpression *expression, int indent)
{
}
static void _print_ast(EntryNode *node, int indent)
{
}
void print_ast(EntryNode *node)
{
}
void print_dict_expression(DictExpression *expression)
{
}

void dd_yyerror(char *errmsg)
{
}

DataDictionary *new_data_dictionary(void)
{
    #if 0
    const int start_size = 28; // size must be >= 1. ---Currently, if the table is not big enough, probing will cause an infinite loop.
    // Be careful allocating for this "variable-sized struct". See Dictionary struct definition.
    DataDictionary *dict = (DataDictionary *) calloc(1, sizeof(DataDictionary) + sizeof(DictionaryTableCell)*(start_size - 1));
    ast_mem_check(dict); //use mem_check when this is a project library.
    dict->table_size = start_size;
    dict->table = &dict->___table; //---- This could be done just by ensuring the allocated memory is contiguous with the metadata struct...
    for (int i = 0; i < dict->table_size; i++) {
        dict->table[i].name = -1; // a -1 name symbol-index denotes an empty cell.
    }
    return dict;
    #else
    return NULL;
    #endif
}

DictExpression *scoped_dictionary_expression(DataDictionary *dict, char *name)
{
    #if 0
    /* Scoping of dictionary names (as they appear in dictionary-expressions) works by having each dictionary hold a pointer
     * to the dictionary it was queried from. When a name appears in a dict-expression of this dictionary, it first searches for a dictionary of that name
     * in itself, then in its parent, then, ..., etc.
     */
    /* printf("Searching for \"%s\"\n", name); */
    DataDictionary *searching_dict = dict;
    while (searching_dict != NULL) {
        DictExpression *found = lookup_dict_expression(searching_dict, name);
        if (found != NULL) return found;
        searching_dict = searching_dict->parent_dictionary;
    }
    printf("Could not find dictionary with name \"%s\" in the current scope.\n", name);
    exit(EXIT_FAILURE);
    /* return NULL; */
    #else
    return NULL;
    #endif
}


// Recursive function and "header" function for evaluating the types of a dictionary. This is done by expanding the expression and building up
// the symbol-numbers of the names of each named dictionary that appears in the expression.
static int ___compute_dictionary_expression_types(DataDictionary *dd, int types[], int index, const int max_num_types, DictExpression *expression)
{
    #if 0
    if (index >= max_num_types) {
        fprintf(stderr, ERROR_ALERT "Too many dictionary types. The maximum number can be increased. It is currently set to %d.\n", max_num_types);
        exit(EXIT_FAILURE);
    }
    while (expression != NULL) {
        if (expression->is_name) {
            // add this operand's name as a type.
            types[index] = expression->name;
            DictExpression *expanding_expression = scoped_dictionary_expression(dd, symbol(expression->name));
            // Recur.
            index = ___compute_dictionary_expression_types(dd, types, index + 1, max_num_types, expanding_expression);
        }
        expression = expression->next;
    }
    return index;
    #else
    return 0;
    #endif
}
static void compute_dictionary_expression_types(DataDictionary *dd, DictionaryTableCell *cell)
{
    #if 0
    if (cell->contents.dict.types != NULL) {
        fprintf(stderr, ERROR_ALERT "Something went wrong. Attempted to compute dictionary expression types for dictionary which already has types.\n");
        exit(EXIT_FAILURE);
    }
    const int max_num_types = 1024;
    int types[max_num_types];
    int num_types = ___compute_dictionary_expression_types(dd, types, 0, max_num_types, cell->contents.dict.dict_expression);

    cell->contents.dict.num_types = num_types;
    cell->contents.dict.types = (int *) calloc(1, sizeof(int) * num_types);
    mem_check(cell->contents.dict.types);
    memcpy(cell->contents.dict.types, types, sizeof(int) * num_types);
    #else
    return;
    #endif
}

static void ___resolve_dictionary_expression(DataDictionary *dict_table, DataDictionary *dict, DictExpression *expression)
{
    #if 0
    // dict: The dictionary the expression is an entry in. This is used for scoping names in the expression.
    //
    // This is the recurred part of the expression resolution. Named operands expand and resolve their named operands and so on, logically leaving
    // the evaluated table as the table evaluated from the expression fully expanded to just a concatenation ( ... ) ( ... ) ( ... ) ... ,
    // although here it is done progressively instead of expanding the expression first.
    while (expression != NULL) {
        if (expression->is_name) {
            // A name references an expression in the current scope. This expands into the expression.
            // For example,
            // ( ... ) Name ( ... ) ===> ( ... ) [expanded Name: ( ... ) AnotherName ( ... ) ( ... )] ( ... )
            DictExpression *expanding_expression = scoped_dictionary_expression(dict, symbol(expression->name));
            // Recur.
            ___resolve_dictionary_expression(dict_table, dict, expanding_expression); // Same dict_table (table that is being filled).
                            //------------------------Does dict have to be changed to where the expression was found? Then scoped_dictionary_expression must return this,
                            //                        so that the expanded expression can evaluate with the correct scope.
        } else {
            // Mask a literal dictionary into the table.
            // note: For an expression to be correctly formed it must terminate to literal dictionaries. If it doesn't, currently infinite loops may be entered.
            if (!mask_dictionary_to_table(dict_table, expression->dict)) {
                fprintf(stderr, "ERROR: Something went wrong when masking a dictionary.\n");
                exit(EXIT_FAILURE);
            }
        }
        expression = expression->next;
    }
    #else
    return;
    #endif
}
DataDictionary *resolve_dictionary_expression(DataDictionary *parent_dd, DictExpression *expression)
{
    #if 1
    DataDictionary *dd = new_data_dictionary();
    dd->parent_dictionary = parent_dd; //give it the parent, so the data-dictionaries form a tree, and scoping can be done.
    ___resolve_dictionary_expression(dd, parent_dd, expression);
    
    // This must be done after a data dictionary is created.
    // For each subdictionary in this dictionary, expand its dictionary-expression and collate the names of named
    // dictionaries, as the dictionary "types".
    // This prepares this data-dictionary for querying by type.
    for (int i = 0; i < dd->table_size; i++) {
        if (dd->table[i].is_dict) {
            compute_dictionary_expression_types(dd, &dd->table[i]);
        }
    }
    return dd;
    #else
    return NULL;
    #endif
}


/*--------------------------------------------------------------------------------
Hash tree A's entries into the table (storing all info about each value-entry and dict-entry).
One by one, hash B's entries into the same table.
value-entries:
    If a dict-entry is matched, this is an error.
    If an entry with the same name is there, check that B_i has no type or the same type. If not, this is an error.
    Overwrite this entry with B_i.
    If an entry without that name is there, write it into the table.
    (--- Should new entries be checked to be sure they declare a type? Or would this restrict usage,
     as you need to make sure you are masking onto something which declares everything? It may be better to just store this as an untyped entry. value-texts are just strings until something tries to
     deserialize them anyway.)
dict-entries:
    If a value-entry is matched, this is an error.
    If an entry with the same name is there, concatenate B_i's dict-expression onto one in the table.
    Otherwise, add this as a new dict-entry.
--------------------------------------------------------------------------------*/
uint32_t crc32(char *string)
{
#if 0
    // This is not crc32 ...
    uint32_t hash = 0;
    int len = strlen(string);
    for (int i = 0; i < len; i++) {
        int n = 1;
        for (int j = 0; j < i; j++) {
            n *= string[i];
        }
        hash += n;
    }
    return hash;
#endif
}
bool mask_dictionary_to_table(DataDictionary *dict_table, EntryNode *dict)
{
}
// Lookup a dictionary-expression in a dictionary.
DictExpression *lookup_dict_expression(DataDictionary *dict, char *name)
{
}
