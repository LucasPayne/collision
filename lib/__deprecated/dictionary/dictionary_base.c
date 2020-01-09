/*--------------------------------------------------------------------------------
  The basis of the dictionary module.

  The basic dictionary key-value pair structure is defined here.
--------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "helper_definitions.h"
#include "dictionary.h"

// Static declarations
//--------------------------------------------------------------------------------
static uint32_t make_hash(char *key);
static char *DictNode_value(DictNode *node);
static char *DictNode_key(DictNode *node);

// Definitions
//--------------------------------------------------------------------------------
static char *DictNode_key(DictNode *node)
{
    return &(node->_string);
}
static char *DictNode_value(DictNode *node)
{
    return &(node->_string) + node->value_pos;
}

static uint32_t make_hash(char *key)
{
    char *c = key;
    uint32_t hash = 0, i = 0;
    /* while (*c++ != '\0' && ++i) hash += (i * *c) % (2 * *c + 256 * i); */
    /* while (*c++ != '\0' && ++i) hash += (i * i * i * i * *c) % (2 * *c + 256 * i); */
    int p = 119569;
    while (*c++ != '\0' && ++i) { int a=1; for (int j=0;j<i-1;j++){a*=123;}hash+=a* *c + i*i*i*i;}
    return hash%p;
}

void dict_add(Dictionary *dictionary, char *key, char *value)
{
    uint32_t hash = make_hash(key) % dictionary->size;

    DictNode *cur = dictionary->table[hash];
    DictNode *last = cur;
    while (cur != NULL) {
        last = cur;
        cur = cur->next;
    }
    DictNode *new = calloc(1, sizeof(DictNode) + strlen(key) + 0 + strlen(value) + 1); // The char entry accounted for in sizeof gives room for an extra null-terminator.
    if (new == NULL) {
        fprintf(stderr, "ERROR: Could not allocate memory for new entry in dictionary.\n");
        exit(EXIT_FAILURE);
    }
    new->next = NULL;
    strcpy(&(new->_string), key);
    new->value_pos = strlen(key) + 1;
    strcpy(&(new->_string) + new->value_pos, value);
    if (last == NULL) {
        dictionary->table[hash] = new;
    } else {
        ((DictNode *) last)->next = new;
    }
}
bool dict_get(Dictionary *dictionary, char *key, char *buffer, size_t buffer_size)
{
    uint32_t hash = make_hash(key) % dictionary->size;
    DictNode *cur = dictionary->table[hash];
    while (cur != NULL) {
        if (strcmp(DictNode_key(cur), key) == 0) {
            if (strlen(DictNode_value(cur)) > buffer_size - 1) return false;
            strncpy(buffer, DictNode_value(cur), buffer_size);
            return true;
        }
        cur = cur->next;
    }
    return false;
}

static void ___free_end_dict_list(DictNode *cur)
{
    if (cur->next != NULL) ___free_end_dict_list(cur->next);
    free(cur);
}
void destroy_dictionary(Dictionary *dictionary)
{
    if (dictionary != NULL) {
        if (dictionary->table != NULL) {
            for (int i = 0; i < dictionary->size; i++) {
                if (dictionary->table[i] != NULL) {
                    ___free_end_dict_list(dictionary->table[i]);
                }
            }
            free(dictionary->table);
        }
        free(dictionary);
    }
}
Dictionary *new_dictionary(uint32_t size)
{
    Dictionary *dic = (Dictionary *) calloc(1, sizeof(Dictionary));
    if (dic == NULL) goto mem_error;
    dic->size = size;
    dic->table = (DictNode **) calloc(size, sizeof(DictNode *));
    if (dic->table == NULL) goto mem_error;
    return dic;
mem_error:
    fprintf(stderr, "ERROR: Failed to allocate memory for dictionary.\n");
    exit(EXIT_FAILURE);
}

void print_dictionary(Dictionary *dictionary)
{
    printf("DICTIONARY\n");
    printf("SIZE: %u\n", dictionary->size);
    for (int i = 0; i < dictionary->size; i++) {
        printf("[");
        DictNode *cur = dictionary->table[i];
        int num = 0;
        while(cur != NULL && ++num) {
            printf("%s[%s]: %s", num==1?"":", ",
                                 DictNode_key(cur),
                                 DictNode_value(cur));
            cur = cur->next;
        }
        printf("]\n");
    }
}
