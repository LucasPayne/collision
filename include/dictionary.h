/*================================================================================
  "Dictionaries", key-value-pair file module. These are not the data structure.
  This module implements a basic file format for key-value storage and querying.
  This is intended to be used for basic configuration and manifest files.

Example:
    thing: val
    // comment
    person: 2222
================================================================================*/
#ifndef HEADER_DEFINED_DICTIONARY
#define HEADER_DEFINED_DICTIONARY
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// The entry order in this struct is important as it is the base of a variable-size structure with a trailing string.
typedef struct DictNode_s {
    void *next;
    // Storing the keys in the nodes. This is so the linked list can be searched for a specific key-value pair. It might be possible
    // to instead store a magic number, and make sure that it is highly unlikely for magic numbers to clash.
    uint16_t value_pos; // Instead of requiring searching for the null-terminator, store the start position of the value string here.
    char _string; // Memory will be allocated here, so &node->_string will be a char *.
                  // This also allows it to be set to \0 if no extra space needs to be allocated.
} DictNode;

typedef struct Dictionary_s {
    uint32_t size;
    DictNode **table;
} Dictionary;
void dict_add(Dictionary *dictionary, char *key, char *value);
bool dict_get(Dictionary *dictionary, char *key, char *buffer, size_t buffer_size);
void print_dictionary(Dictionary *dictionary);
Dictionary *new_dictionary(uint32_t size);
void destroy_dictionary(Dictionary *dictionary);

// Lexer dictionary__read.l
Dictionary *dictionary_read(FILE *file);

/*--------------------------------------------------------------------------------
  Querying
--------------------------------------------------------------------------------*/
typedef struct DictQuerier_s {
    Dictionary *dictionary;
    int num_rules;

    char **type_names;
    bool (**query_val_functions)(char *, void *); //e.g. bool query_val_vec4(char *string, void *val);
} DictQuerier;

void dict_query_rule_add(DictQuerier *q, char *type_name, bool (*query_val_function)(char *, void *));
DictQuerier *dict_new_querier(Dictionary *dictionary);
bool dict_query_get(DictQuerier *q, char *type_name, char *entry_name, void *data);

#endif // HEADER_DEFINED_DICTIONARY
