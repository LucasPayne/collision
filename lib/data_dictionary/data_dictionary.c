/*--------------------------------------------------------------------------------
note: it is not neccessary to have a separate C source file for the interface,
just a separate header. But this may be nicer for editing the module.
--------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "data_dictionary.h"
#include "data_dictionary_implementation.h"

DataDictionary *dd_fopen(char *path)
{
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        printf("Could not find data-dictionary file \"%s\".\n", path);
        return NULL;
    }
    dd_push_file(file);
    dd_yyparse();
    
    // Formulate the top-level dictionary as a 1-operand dict-expression, then open it.
    DictExpression top_expression = { 0 };
    top_expression.is_name = false;
    top_expression.dict = g_dict; // g_dict has been left by the parser as the top-level dictionary IR.
    DataDictionary *dict = resolve_dictionary_expression(NULL, &top_expression);
    if (dict == NULL) {
        printf("Something went wrong when trying to open data-dictionary file \"%s\".\n", path);
        return NULL;
    }
    return dict;
}

DataDictionary *dd_open(DataDictionary *dict, char *name)
{
    DictExpression *expression = lookup_dict_expression(dict, name);
    if (expression == NULL) return NULL;
    DataDictionary *found_dict = resolve_dictionary_expression(dict, expression);
    // Give it a pointer to the queried dictionary, for scoping purposes. This means that queried-for dictionaries
    // form a tree of dictionary tables.
    found_dict->parent_dictionary = dict;
    return found_dict;
}

// Lookup a value in a dictionary.
bool dd_get(DataDictionary *dict, char *name)
{
    uint32_t hash = crc32(name);
    int index = hash % dict->table_size;
    while (dict->table[index].name != -1) {
        if (strcmp(name, symbol(dict->table[index].name)) == 0) {
            if (dict->table[index].is_dict) {
                printf("ERROR dd_get: Attempted to extract dictionary-entry \"%s\" from dictionary as a value.\n", name);
                return false;
            }
            // Dictionary-entry found with the given name, open it as a table and return the table.
            printf("Found value for %s: %s\n", name, symbol(dict->table[index].contents.value.value_text));
            return true;
        }
        //note: Make sure this mirrors the hashing and indexing done when the table is created.
        index = (index + 1) % dict->table_size;
    }
    printf("ERROR dd_get: Entry \"%s\" not found in dictionary.\n", name);
    return false;
}

void dd_print_table(DataDictionary *dict_table)
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



