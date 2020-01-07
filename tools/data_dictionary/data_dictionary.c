#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
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

DictExpression *new_named_dict_expression(int name_symbol)
{
    DictExpression *expression = (DictExpression *) calloc(1, sizeof(DictExpression));
    ast_mem_check(expression);
    expression->is_name = true;
    expression->name = name_symbol;
    return expression;
}
DictExpression *new_literal_dict_expression(EntryNode *dict)
{
    DictExpression *expression = (DictExpression *) calloc(1, sizeof(DictExpression));
    ast_mem_check(expression);
    expression->is_name = false;
    expression->dict = dict;
    return expression;
}


static void _print_dict_expression(DictExpression *expression, int indent);
static void _print_ast(EntryNode *node, int indent);

static void _print_dict_expression(DictExpression *expression, int indent)
{
    while (expression != NULL) {
        for (int i = 0; i < indent*2; i++) putchar(' ');
        printf("+ ");
        if (expression->is_name) {
            printf("%s\n", symbol(expression->name));
        } else {
            printf("dictionary-literal\n");
            _print_ast(expression->dict, indent + 1);
        }
        expression = expression->next;
    }
}
static void _print_ast(EntryNode *node, int indent)
{
    while (node != NULL) {
        for (int i = 0; i < indent*2; i++) putchar(' ');
        printf("%s-> [", symbol(node->name));
        if (node->is_dict) {
            printf("Kind: Dict]\n");
            _print_dict_expression(node->dict_expression, indent + 1);
        } else {
            printf("Kind: Entry, Type: %s, Value: %s]\n", node->type == -1 ? "NONE" : symbol(node->type),
                                                          node->value_text == -1 ? "NONE" : symbol(node->value_text));
        }
        node = node->next;
    }
}
void print_ast(EntryNode *node)
{
    _print_ast(node, 0);
}
void print_dict_expression(DictExpression *expression)
{
    _print_dict_expression(expression, 0);
}

void yyerror(char *errmsg)
{
    fprintf(stderr, "Parsing error: %s\n", errmsg);
    exit(EXIT_FAILURE);
}

// void dict_table_mask(DictionaryTable *table, Dictionary *dict)
// {
//     EntryNode *entry = (EntryNode *) dict;
//     while (entry != NULL) {
//         uint32_t hash = crc32(symbol(entry->name));
//         table->entries[hash % table->entries_size];
// 
//         entry = entry->next;
//     }
// }
// 
// Dictionary *resolve_dict_expression(DictExpression *expression)
// {
//     /* From left to right, successively mask the dictionary.
//      * If a new value-entry is added which does not have a type, the resolution fails.
//      * If a value-entry has the same name but a different type (or different kind, dict/value) as a previous entry, the resolution fails.
//      */
//     DictionaryTable *dict_table = new_dictionary_table();
//     while (expression != NULL) {
//         Dictionary *masking_dict;
//         if (expression->is_name) {
//             masking_dict = get_dict_from_name(expression->name);
//             if (masking_dict == NULL) {
//                 printf("Error in dictionary expression: \"%s\" is not the name of a dictionary.\n", symbol(expression->name));
//                 return NULL;
//             }
//         } else {
//             masking_dict = expression->dict;
//         }
//         dict_table_mask(dict_table, masking_dict);
//         expression = expression->next;
//     }
//     return NULL;
// }
// 
// Dictionary *dict_get_dict(Dictionary *dict, char *name)
// {
//     EntryNode *entry = (EntryNode *) dict;
//     while (entry != NULL) {
//         if (entry->is_dict && strcmp(name, symbol(entry->name)) == 0) {
//             printf("found\n");
//             return resolve_dict_expression(entry->dict_expression);
//         }
//         entry = entry->next;
//     }
//     printf("not found\n");
//     return NULL;
// }

Dictionary *new_dictionary(void)
{
    const int start_size = 28; // size must be >= 1.
    // Be careful allocating for the the "variable-size struct". See Dictionary struct definition.
    Dictionary *dict = (Dictionary *) calloc(1, sizeof(Dictionary) + sizeof(DictionaryTableCell)*(start_size - 1));
    ast_mem_check(dict); //use mem_check when this is a project library.
    dict->table_size = start_size;
    dict->table = &dict->___table; // The pointer entry is then just a nicety.
                                   //---- This could be done just by ensuring the allocated memory is contiguous with the metadata struct...
    for (int i = 0; i < dict->table_size; i++) {
        dict->table[i].name = -1; // a -1 name symbol-index denotes an empty cell.
    }
    return dict;
}
Dictionary *open_dictionary(DictExpression *expression)
{
    // The DictExpression pointer is the start of the linked-list IR dictionary as a dict-expression, which when "opened", creates a new table and successively
    // masks each operand into the table.
    Dictionary *dict_table = new_dictionary();
    while (expression != NULL) {
        if (expression->is_name) {
            // look for dictionary in the table.
        } else {
            if (!mask_dictionary_to_table(dict_table, expression->dict)) {
                fprintf(stderr, "ERROR: Something went wrong when masking a dictionary.\n");
                exit(EXIT_FAILURE);
            }
        }
        expression = expression->next;
    }
    return dict_table;
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
}

bool mask_dictionary_to_table(Dictionary *dict_table, EntryNode *dict)
{
    // A EntryNode is the IR, linked-list representation of a dictionary. This is what it is kept as in memory until
    // a dictionary is needed as part of a dict-expression (or when the file is read, the root dictionary is treated as a 1-operand dict-expression),
    // where it is read into a table, to do the masking operation and type-checking.
    #define mask_error(STRING)\
    {\
        printf("Error when masking dictionary to table: " STRING "\n");\
        return false;\
    }
    EntryNode *entry = dict;
    // Successively add entries to the table.
    while (entry != NULL) {
        uint32_t hash = crc32(symbol(entry->name));
        int index = hash % dict_table->table_size;
        bool appended_expression = false; // This is set to true in the loop if the dict-entry has masked by appending its expression onto the other expression.
                                          // At the end of the loop, if this is false, a new dict-entry is created at the index instead.
        // Probe until an empty cell or a name-match is found.
        while (dict_table->table[index].name != -1) { // Closed addressing.
            if (strcmp(symbol(entry->name), symbol(dict_table->table[index].name)) == 0) { // could store and compare crc32 hashes, but this works.
                // A name match has been found. Proceed to do type-checking and then masking.
                DictionaryTableCell *other_entry = &dict_table->table[index];
                if (entry->is_dict) {
                    if (!other_entry->is_dict) mask_error("Attempted to override a value-entry with a dictionary-entry.");
                    if (other_entry->contents.dict.dict_expression == NULL) {
                        // Old entry has an empty expression. Just give it the new expression.
                        other_entry->contents.dict.dict_expression = entry->dict_expression;
                    } else {
                        // Concatenate their expressions.
                        // First, go to the end, then set its next-pointer to the start of the appended expression.
                        DictExpression *end = other_entry->contents.dict.dict_expression;
                        while (end->next != NULL) end = end->next;
                        end->next = entry->dict_expression;
                    }
                    // Break out of the loop, but first set this flag, to signify that a succesful dict-entry mask has taken place.
                    appended_expression = true;
                    break;
                } else {
                    if (other_entry->is_dict) mask_error("Attempted to override a dictionary-entry with a value-entry.");
                    // Type check. Invalid cases:
                    //                      A typed value is written into a non-typed value.
                    //                      A typed value is written into a typed-value with a different type.
                    if (other_entry->contents.value.type == -1 && entry->type != -1) mask_error("Attempted to overwrite a non-typed value with a typed value.");
                    if (entry->type != -1 && other_entry->contents.value.type != -1 && strcmp(symbol(entry->type), symbol(other_entry->contents.value.type)) != 0) {
                        mask_error("Attempted to overwrite a typed value with a typed value of a different type.");
                    }
                    // Break out of the loop. The value will be overwritten at the cell at this index in the same way that
                    // a new value is added at an empty cell.
                    break;
                }
            }
            index = (index + 1) % dict_table->table_size; // Linear probing.
        }
        if (entry->is_dict && !appended_expression) {
            // A new dict-entry has been added, but it has not masked onto a previous one. So, add it.
            dict_table->table[index].name = entry->name;
            dict_table->table[index].is_dict = true;
            dict_table->table[index].contents.dict.dict_expression = entry->dict_expression;
        }
        if (!entry->is_dict) {
            // add a new value-entry, or overwrite a previous one.
            dict_table->table[index].name = entry->name;
            dict_table->table[index].is_dict = false;
            dict_table->table[index].contents.value.type = entry->type;
            dict_table->table[index].contents.value.value_text = entry->value_text;
        }
        entry = entry->next;
    }
    return true;
    #undef mask_error
}

void print_dict_table(Dictionary *dict_table)
{
    printf("---dictionary table-------------------------------------------------------------\n");
    for (int i = 0; i < dict_table->table_size; i++) {
        DictionaryTableCell *cell = &dict_table->table[i];
        printf("%d: ", i);
        if (cell->name == -1) {
            // empty cell.
            printf("___\n");
            continue;
        }
        if (cell->is_dict) {
            printf("DICT %s-> ", symbol(cell->name));
            DictExpression *operand = cell->contents.dict.dict_expression;
            int count = 0;
            while (operand != NULL) {
                if (count++ > 0) printf("+ ");
                if (operand->is_name) printf("%s ", symbol(operand->name));
                else {
                    printf("DICT-LITERAL ");
                }
                operand = operand->next;
            }
            printf("\n");
        } else {
            printf("%s %s: %s\n", cell->contents.value.type == -1 ? "NOTYPE" : symbol(cell->contents.value.type), symbol(cell->name),
                                  cell->contents.value.value_text == -1 ? "NOVALUE" : symbol(cell->contents.value.value_text));
        }
    }
}



// Lookup a dictionary in a dictionary.
Dictionary *lookup_dict(Dictionary *dict, char *name)
{
    printf("Looking up dictionary \"%s\" ...\n", name);
    uint32_t hash = crc32(name);
    int index = hash % dict->table_size;
    while (dict->table[index].name != -1) {
        if (strcmp(name, symbol(dict->table[index].name)) == 0) {
            if (!dict->table[index].is_dict) {
                printf("ERROR lookup_dict: Attempted to extract value-entry \"%s\" from dictionary as a dictionary.\n", name);
                return NULL;
            }
            // Dictionary-entry found with the given name, open it as a table and return the table.
            return open_dictionary(dict->table[index].contents.dict.dict_expression);
        }
        //note: Make sure this mirrors the hashing and indexing done when the table is created.
        index = (index + 1) % dict->table_size;
    }
    printf("ERROR lookup_dict: Entry \"%s\" not found in dictionary.\n", name);
    return NULL;
}

bool lookup_value(Dictionary *dict, char *name)
{
    printf("Looking up value \"%s\" ...\n", name);
    uint32_t hash = crc32(name);
    int index = hash % dict->table_size;
    while (dict->table[index].name != -1) {
        if (strcmp(name, symbol(dict->table[index].name)) == 0) {
            if (dict->table[index].is_dict) {
                printf("ERROR lookup_value: Attempted to extract dictionary-entry \"%s\" from dictionary as a value.\n", name);
                return false;
            }
            // Dictionary-entry found with the given name, open it as a table and return the table.
            return open_dictionary(dict->table[index].contents.dict.dict_expression);
            printf("Found value for %s: %s\n", name, symbol(dict->table[index].contents.value.value_text));
            return true;
        }
        //note: Make sure this mirrors the hashing and indexing done when the table is created.
        index = (index + 1) % dict->table_size;
    }
    printf("ERROR lookup_value: Entry \"%s\" not found in dictionary.\n", name);
    return false;
}


int main(void)
{
    yyin = fopen("tests/test1", "r");
    yyparse();
    print_ast(g_dict);

    // Formulate the top-level dictionary as a 1-operand dict-expression, then open it.
    DictExpression top_expression = { 0 };
    top_expression.is_name = false;
    top_expression.dict = g_dict; // g_dict has been left by the parser as the top-level dictionary IR.

    Dictionary *dict = open_dictionary(&top_expression);
    print_dict_table(dict);

    Dictionary *subdict = lookup_dict(dict, "Spider");
    print_dict_table(subdict);
    {
        Dictionary *subsubdict = lookup_dict(subdict, "transform");
        print_dict_table(subsubdict);
    }
    {
        Dictionary *subsubdict = lookup_dict(subdict, "body");
        print_dict_table(subsubdict);
        Dictionary *subsubsubdict = lookup_dict(subsubdict, "material");
        print_dict_table(subsubsubdict);
    }
}



