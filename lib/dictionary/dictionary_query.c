/*--------------------------------------------------------------------------------
  The querying part of the dictionary module.
--------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "helper_definitions.h"
#include "dictionary.h"

void dict_query_rule_add(DictQuerier *q, char *type_name, bool (*query_val_function)(char *, void *))
{
    /* dict_query_rule_add(q, "ResourcePath", query_val_ResourcePath); */

    // Grow the array just enough to fit the new rule.
    if (q->type_names == NULL) {
        q->num_rules = 1;
        q->type_names = (char **) malloc(q->num_rules * sizeof(char *));
        mem_check(q->type_names);
        q->query_val_functions = (bool (**)(char *, void *)) malloc(q->num_rules * sizeof(bool (*)(char *, void *)));
        mem_check(q->query_val_functions);
    } else {
        q->num_rules ++;
        q->type_names = (char **) realloc(q->type_names, q->num_rules * sizeof(char *));
        mem_check(q->type_names);
        q->query_val_functions = (bool (**)(char *, void *)) realloc(q->query_val_functions, q->num_rules * sizeof(bool (*)(char *, void *)));
        mem_check(q->query_val_functions);
    }
    // Give the query validating/value getting function to the querier for the new rule.
    q->query_val_functions[q->num_rules - 1] = query_val_function;
    // Transfer the type name given to the heap, and give this to the querier for the new rule.
    char *type_name_on_heap = (char *) malloc((strlen(type_name) + 1) * sizeof(char));
    mem_check(type_name_on_heap);
    strcpy(type_name_on_heap, type_name);
    q->type_names[q->num_rules - 1] = type_name_on_heap;
}

DictQuerier *dict_new_querier(Dictionary *dictionary)
{
    if (dictionary == NULL) {
        fprintf(stderr, ERROR_ALERT "Attempted to create a new dictionary querier with a null dictionary pointer.\n");
        exit(EXIT_FAILURE);
    }
    DictQuerier* querier = (DictQuerier *) calloc(1, sizeof(DictQuerier));
    mem_check(querier);

    querier->dictionary = dictionary;
    return querier;
}

bool dict_query_get(DictQuerier *q, char *type_name, char *entry_name, void *data)
{
    bool (*validator)(char *, void *);
    int i;
    for (i = 0; i < q->num_rules; i++) {
        if (strcmp(type_name, q->type_names[i]) == 0) {
            validator = q->query_val_functions[i];
            break;
        }
    }
    if (i == q->num_rules) {
        fprintf(stderr, ERROR_ALERT "Attempted to query a dictionary and validate with type name \"%s\".\
The dictionary is not set up to validate this type name.\n");
        exit(EXIT_FAILURE);
    }
    const int buf_size = 1024;
    char buf[buf_size];
    if (!dict_get(q->dictionary, entry_name, buf, buf_size)) return false;
    if (!validator(buf, data)) return false;
    return true;
}


/*--------------------------------------------------------------------------------
void *GraphicsProgram_load(char *resource_path)
{
    char path[1024];
    if (!resource_file_path(resource_path, ".GraphicsProgram", path, 1024)) return NULL;
    FILE *file = fopen(path, "r");
    if (file == NULL) return NULL;
    Dictionary *dictionary = read_dictionary(file);
    if (dictionary == NULL) return NULL;
    
    DictQuerier *q = dict_new_querier(dictionary);
    dict_query_rules_rendering(q);

    VertexFormat vertex_format;
    if (!dict_query_get(q, "VertexFormat", "vertex_format", &vertex_format, sizeof(vertex_format))) return NULL;
    const int buf_size = 1024;
    char vertex_shader_resource_path[buf_size];
    char fragment_shader_resource_path[buf_size];
    if (!dict_query_get_string(q, "vertex_shader", vertex_shader_resource_path, buf_size)) return NULL;
    if (!dict_query_get_string(q, "fragment_shader", fragment_shader_resource_path, buf_size)) return NULL;

    // ... do stuff with these values

    dict_destroy_querier(q);
}
--------------------------------------------------------------------------------*/

