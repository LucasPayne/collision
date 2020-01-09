/*--------------------------------------------------------------------------------
note: It is not neccessary to have a separate C source file for the interface,
just a separate header. But this may be nicer for editing the module.
--------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include "helper_definitions.h"
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
bool dd_get(DataDictionary *dict, char *name, char *type, void *data)
{
    uint32_t hash = crc32(name);
    int index = hash % dict->table_size;
    while (dict->table[index].name != -1) {
        if (strcmp(name, symbol(dict->table[index].name)) == 0) {
            if (dict->table[index].is_dict) {
                printf("ERROR dd_get: Attempted to extract dictionary-entry \"%s\" from dictionary as a value.\n", name);
                return false;
            }
            if (strcmp(symbol(dict->table[index].contents.value.type), type) != 0) {
                printf("ERROR dd_get: Unexpected type, attempted to extract \"%s %s\" when \"%s\"'s type is \"%s\".\n", type, name, name, symbol(dict->table[index].contents.value.type));
                return false;
            }
            DDTypeReader reader = dd_get_reader(type);
            if (reader == NULL) {
                printf("ERROR dd_get: No type reader for type \"%s\".\n", type);
                return false;
            }
            // Use the type-reader to parse the value-text.
            return reader(symbol(dict->table[index].contents.value.value_text), data);
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
            printf("DICT %s < ", symbol(cell->name));
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
            printf("[types: ");
            for (int i = 0; i < cell->contents.dict.num_types; i++) {
                if (i > 0) printf(", ");
                printf("%s", symbol(cell->contents.dict.types[i]));
            }
            printf("]\n");
        } else {
            printf("%s %s: %s\n", cell->contents.value.type == -1 ? "NOTYPE" : symbol(cell->contents.value.type), symbol(cell->name),
                                  cell->contents.value.value_text == -1 ? "NOVALUE" : symbol(cell->contents.value.value_text));
        }
    }
}


#define DD_TYPE_READER(TYPE) bool dd_type_reader_ ## TYPE(const char *text, void *data)

DD_TYPE_READER(bool) {
    if (strcmp(text, "true")  == 0) { *((bool *) data) = true;  return true; }
    if (strcmp(text, "false") == 0) { *((bool *) data) = false; return true; }
    return false;
}
DD_TYPE_READER(int) {
    //---prevent trailing string stuff.
    int val;
    if (sscanf(text, "%d", &val) == EOF) return false;
    *((int *) data) = val;
    return true;
}
DD_TYPE_READER(vec4) {
    // 4 floats, 16 bytes.
    // format: 0, 1, 2, 3
    // [0]
    // [1]
    // [2]
    // [3]
    char *p = text;
    float v[4];
    int chars_read;
    for (int i = 0; i < 4; i++) {
        if (sscanf(p, (i < 3) ? "%f%n," : "%f%n", &v[i], &chars_read) == EOF) return false;
        p += chars_read + 1;
    }
    memcpy(data, &v, sizeof(float)*4);
    return true;
}

#define type(TYPE) { dd_type_reader_ ## TYPE, #TYPE }
const struct { DDTypeReader reader; const char *type_name; } dd_type_readers[] = {
    type(bool),
    type(int),
    type(vec4),
    { NULL, NULL }
};
#undef type

DDTypeReader dd_get_reader(const char *type)
{
    int i = 0;
    while (dd_type_readers[i].reader != NULL && dd_type_readers[i].type_name != NULL) {
        if (strcmp(type, dd_type_readers[i].type_name) == 0) {
            return dd_type_readers[i].reader;
        }
        i++;
    }
    return NULL;
}



    /* DD *scene = dd_open(dd, "Scene"); */
    /* DD **spiders; */
    /* int num = dd_scan(scene, &spiders, "Spider"); */
    /* for (int i = 0; i < num; i++) { */
    /*     dd_print_table(spiders[i]); */
    /* } */

int dd_scan(DataDictionary *dd, DataDictionary ***scanned, const char *type_string)
{
    // Go through the dictionary and set *scanned to be an array of DataDictionary pointers to dictionaries
    // which match a type string.
    // A dictionary has a list of types, one for each named dictionary it derives from (in its expanded dictionary-expression).
    // Returns the number of dictionaries scanned, and -1 if there was an error.
    // Currently, type_string is interpreted only as a single type.

    const int max_num_scanned = 1024;
    int count = 0;
    int to_open[max_num_scanned];

    for (int i = 0; i < dd->table_size; i++) {
        if (dd->table[i].is_dict) {
            // Try to match the type.
            for (int j = 0; j < dd->table[i].contents.dict.num_types; j++) {
                if (strcmp(type_string, symbol(dd->table[i].contents.dict.types[j])) == 0) {
                    if (count >= max_num_scanned) {
                        fprintf(stderr, ERROR_ALERT "dd_scan: Encountered too many dictionaries. The maximum is set to %d, and this can be increased.\n", max_num_scanned);
                        exit(EXIT_FAILURE);
                    }
                    to_open[count ++] = i;
                    break;
                }
            }
        }
    }
    DataDictionary **opened_dds = (DataDictionary **) calloc(1, sizeof(DataDictionary *) * count);
    mem_check(opened_dds);
    for (int i = 0; i < count; i++) {
        // Go through the scanned indices, and open each dictionary by resolving the expression at that index.
        DictExpression *expression = dd->table[to_open[i]].contents.dict.dict_expression;
        opened_dds[i] = resolve_dictionary_expression(dd, expression);
        opened_dds[i]->parent_dictionary = dd;
    }
    *scanned = opened_dds;
    return count;
}


void dd_print(DataDictionary *dd)
{

}

