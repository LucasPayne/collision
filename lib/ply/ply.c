/*--------------------------------------------------------------------------------
    Definitions and static data for the PLY file format module.
--------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "helper_definitions.h"
#include "ply.h"

static char _ply_format_names[NUM_PLY_FORMATS][28] = { // do not shuffle these!
    "none",
    "ascii 1.0",
    "binary_little_endian 1.0",
    "binary_big_endian 1.0",
};
static char _ply_type_names[NUM_PLY_TYPES][7] = { // do not shuffle these!
    "char",
    "uchar",
    "short",
    "ushort",
    "int",
    "uint",
    "float",
    "double"
};
static size_t _ply_type_sizes[NUM_PLY_TYPES] = { // do not shuffle these!
    1,
    1,
    2,
    2,
    4,
    4,
    4,
    8
};
static bool ply_float_type(PLYType type)
{
    return type == PLY_FLOAT || type == PLY_DOUBLE;
}
static bool ply_int_type(PLYType type)
{
    return type == PLY_CHAR || type == PLY_SHORT || type == PLY_INT;
}
static bool ply_uint_type(PLYType type)
{
    return type == PLY_UCHAR || type == PLY_USHORT || type == PLY_UINT;
}

static PLYType match_ply_type(char *string);

static PLYType match_ply_type(char *string)
{
    for (int i = 0; i < NUM_PLY_TYPES; i++) {
        if (strcmp(string, _ply_type_names[i]) == 0) {
            return i;
        }
    }
    return NULL_PLY_TYPE;
}


//--------------------------------------------------------------------------------
// Initializers and destructors
//--------------------------------------------------------------------------------
void init_ply(PLY *ply)
{
    ply->name = NULL;
    ply->format = PLY_FORMAT_NONE;
    ply->num_elements = 0;
    ply->first_element = NULL;
}
void destroy_ply(PLY *ply)
{
    if (ply->filename != NULL) free(ply->filename);
    if (ply->first_element != NULL) {
        PLYElement *cur_element = ply->first_element;
        do {
            PLYElement *destroy_this = cur_element;
            cur_element = cur_element->next;
            destroy_ply_element(destroy_this);
        } while (cur_element != NULL);
    }
    free(ply);
}
void init_ply_element(PLYElement *ply_element)
{
    ply_element->name = NULL;
    ply_element->count = 0;
    ply_element->num_properties = 0;
    ply_element->next_element = NULL;
    ply_element->first_property = NULL;
}
void destroy_ply_element(PLYElement *ply_element)
{
    if (ply_element->name != NULL) free(ply_element->name);
    if (ply_element->first_property != NULL) {
        PLYProperty *cur_property = ply_element->first_property;
        do {
            PLYProperty *destroy_this = cur_property;
            cur_property = cur_property->next;
            destroy_ply_property(destroy_this);
        } while (cur_property != NULL);
    }
    free(ply_element);
}
void init_ply_property(PLYProperty *ply_property)
{
    ply_property->name = NULL;
    ply_property->type = NULL_PLY_TYPE;
    ply_property->is_list = false;
    ply_property->list_count_type = NULL_PLY_TYPE;
    ply_property->next_propety = NULL;
}
void destroy_ply_property(PLYProperty *ply_property)
{
    if (ply_property->name != NULL) free(ply_property->name);
    free(ply_property);
}
//--------------------------------------------------------------------------------
// Printers and serializers
//--------------------------------------------------------------------------------
void print_ply(PLY *ply)
{
    printf("PLY object:\n");
    printf("filename: %80s\n", ply->filename);
    printf("num_elements: %d\n", ply->num_elements);
    printf("format: %80s (%d)\n", _ply_format_names[ply->format], ply->format);
    printf("Elements:\n");
    if (ply->first_element == NULL) {
        printf("no elements\n");
    } else {
        PLYElement *print_this = ply->first_element;
        do {
            print_ply_element(print_this);
            print_this = print_this->next_element;
        } while (print_this != NULL);
    }
}
void print_ply_element(PLYElement *ply_element)
{
    printf("name: %80s\n", ply_element->name);
    printf("count: %d\n", ply_element->count);
    printf("num_properties: %d\n", ply_element->num_properties);
    printf("Properties:\n");
    if (ply_element->first_property == NULL) {
        printf("no properties\n");
    } else {
        PLYProperty *print_this = ply_element->first_property;
        do {
            print_ply_property(print_this);
            print_this = print_this->next_property;
        } while (print_this != NULL);
    }
}
void print_ply_property(PLYProperty *ply_property)
{

}

typedef struct PLY_s {
    char *filename;
    PLYFormat format;
    int num_elements;
    PLYElement *first_element;
} PLY;
typedef struct PLYElement_s {
    char *name;
    int count;
    int num_properties;
    struct PLYElement_s *next_element;
    PLYProperty *first_property;
} PLYElement;
typedef struct PLYProperty_s {
    char *name;
    PLYType type;
    bool is_list;
    PLYType list_count_type;
    struct PLYProperty_s *next_property;
} PLYProperty;
