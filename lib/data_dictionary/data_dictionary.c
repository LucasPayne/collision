/*--------------------------------------------------------------------------------
note: It is not neccessary to have a separate C source file for the interface,
just a separate header. But this may be nicer for editing the module.
--------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

static DataDictionary *___dd_open(DataDictionary *dict, char *name)
{
    // Non-recursive opening.
    DictExpression *expression = lookup_dict_expression(dict, name, NULL);
    if (expression == NULL) return NULL;
    DataDictionary *found_dict = resolve_dictionary_expression(dict, expression);
    // Give it a pointer to the queried dictionary, for scoping purposes. This means that queried-for dictionaries
    // form a tree of dictionary tables.
    found_dict->parent_dictionary = dict;
    return found_dict;
}
DataDictionary *dd_open(DataDictionary *dict, char *path)
{
    // Recursive opening. Follow the /-separated path. Each scope is checked, starting with the deepest.
    DD *scope_dict = dict;
    printf("Opening \"%s\"\n", path);

    int c = 0;
    while (scope_dict != NULL) {
        char *p = path;
        DataDictionary *cur_dict = scope_dict;
        const int n = 4096;
        char buf[n];
        while (1) {
            char *sep = strchr(p, '/');
            bool finish = false;
            if (sep == NULL) { // Last entry in the path.
                finish = true;
                sep = strchr(p, '\0');
            }
            if (sep - p >= n) {
                fprintf(stderr, "Entry in data-dictionary path queried is too long.\n");
                exit(EXIT_FAILURE);
            }
            strncpy(buf, p, sep - p);
            buf[sep - p] = '\0';
            cur_dict = ___dd_open(cur_dict, buf);
            if (cur_dict != NULL && finish) {
                return cur_dict;
            }
            if (cur_dict == NULL || finish) break; // continue searching up a scope.
            p = sep + 1;
        }
        scope_dict = scope_dict->parent_dictionary;
        ///////not closing dictionaries.
    }
    // Failed to find in any scope.
    return NULL;
}    


// Lookup a value in a dictionary.
bool dd_get(DataDictionary *dict, char *name, char *type, void *data)
{
    uint32_t hash = hash_crc32(name);
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
DD_TYPE_READER(uint) {
    unsigned int val;
    if (sscanf(text, "%u", &val) == EOF) return false;
    *((unsigned int *) data) = val;
    return true;
}
DD_TYPE_READER(float) {
    float val;
    if (sscanf(text, "%f", &val) == EOF) return false;
    *((float *) data) = val;
    return true;
}

// This string needs to be freed by the retriever.
DD_TYPE_READER(string) {
    int len = strlen(text);
    char *str = (char *) malloc(sizeof(char) * (len + 1));
    strcpy(str, text);
    str[len] = '\0';
    *((char **) data) = str;
    return true;
}

static bool read_comma_separated_C_type(const char *text, void *data, const char *fmt, const size_t size, const int count)
{
    char *p = text;
    uint8_t v_data[count * size];
    int chars_read;

    char fmt_n_comma[8];
    char fmt_n[8];
    sprintf(fmt_n_comma, "%%%s%%n,", fmt);
    sprintf(fmt_n, "%%%s%%n", fmt);
    for (int i = 0; i < count; i++) {
        if (sscanf(p, (i < 3) ? fmt_n_comma : fmt_n, &v_data[i*size], &chars_read) == EOF) return false;
        p += chars_read + 1;
    }
    memcpy(data, v_data, count * size);
    return true;
}

DD_TYPE_READER(vec2) { return read_comma_separated_C_type(text, data, "f", sizeof(float), 2); }
DD_TYPE_READER(vec3) { return read_comma_separated_C_type(text, data, "f", sizeof(float), 3); }
DD_TYPE_READER(vec4) { return read_comma_separated_C_type(text, data, "f", sizeof(float), 4); }
DD_TYPE_READER(ivec2) { return read_comma_separated_C_type(text, data, "d", sizeof(int), 2); }
DD_TYPE_READER(ivec3) { return read_comma_separated_C_type(text, data, "d", sizeof(int), 3); }
DD_TYPE_READER(ivec4) { return read_comma_separated_C_type(text, data, "d", sizeof(int), 4); }

#define type(TYPE) { dd_type_reader_ ## TYPE, #TYPE }
const struct { DDTypeReader reader; const char *type_name; } dd_type_readers[] = {
    type(bool),
    type(int),
    type(float),
    type(uint),
    type(vec2),
    type(vec3),
    type(vec4),
    type(ivec2),
    type(ivec3),
    type(ivec4),
    type(string),
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
        if (dd->table[i].name != -1 && dd->table[i].is_dict) {
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
    printf("---dictionary-------------------------------------------------------------------\n");
    for (int i = 0; i < dd->table_size; i++) {
        DictionaryTableCell *cell = &dd->table[i];
        if (cell->name == -1) {
            continue;
        }
        if (cell->is_dict) {
            printf("%s < ", symbol(cell->name));
            DictExpression *operand = cell->contents.dict.dict_expression;
            int count = 0;
            while (operand != NULL) {
                if (count++ > 0) printf("+ ");
                if (operand->is_name) printf("%s ", symbol(operand->name));
                else {
                    printf("LITERAL ");
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
    printf("--------------------------------------------------------------------------------\n");
}

