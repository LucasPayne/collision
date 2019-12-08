#ifndef HEADER_DEFINED_DICT
#define HEADER_DEFINED_DICT

// The entry order in this struct is important as it is the base of a variable-size structure with a trailing string.
typedef struct DictNode_s {
    void *next;
    // Storing the keys in the nodes. This is so the linked list can be searched for a specific key-value pair. It might be possible
    // to instead store a magic number, and make sure that it is highly unlikely for magic numbers to clash.
    uint16_t value_pos;
    char _string; // Memory will be allocated here, so &node->string will be a char *. This also allows it to be set to \0 if no memory needs to be allocated.
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

#endif // HEADER_DEFINED_DICT
