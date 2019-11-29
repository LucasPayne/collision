/*--------------------------------------------------------------------------------
    Definitions for the PLY file format module.
--------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "helper_definitions.h"
#include "text_processing.h"
#include "data/ply.h"

/*--------------------------------------------------------------------------------
Usage:
    PLYStats stats;
    if (!ply_stat("...", &stats)) {
        ... handle bad/misunderstood PLY file
    }
    ... work with stats info to e.g. read ply data into buffers
--------------------------------------------------------------------------------*/

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


// Log this somewhere, not stderr?
#define STAT_ERROR(MSG)\
{\
    printf("STAT ERROR: \n");\
    printf((  MSG ));\
    printf("\n");\
    print_ply_stats(ply_stats);\
    return false;\
}
bool ply_stat(FILE *file, PLYStats *ply_stats)
{
    memset(ply_stats, 0, sizeof(PLYStats)); // ?

    if (!try_read_line(file, "ply")) {
        STAT_ERROR("Bad magic number");
    }
    if (try_read_line(file, "format ascii 1.0")) {
        ply_stats->format = PLY_FORMAT_ASCII_1;
    }
    else if (try_read_line(file, "format binary 1.0")) {
        ply_stats->format = PLY_FORMAT_BINARY_LITTLE_ENDIAN_1; // default?
    }
    else if (try_read_line(file, "format binary_little_endian 1.0")) {
        ply_stats->format = PLY_FORMAT_BINARY_LITTLE_ENDIAN_1;
    }
    else if (try_read_line(file, "format binary_big_endian 1.0")) {
        ply_stats->format = PLY_FORMAT_BINARY_BIG_ENDIAN_1;
    }
    else {
        STAT_ERROR("Invalid format");
    }

    bool getting_properties = false;
    PLYElement *cur_element = &ply_stats->elements[0];
    PLYProperty *cur_property = &cur_element->properties[0];

    while (!try_read_line(file, "end_header")) {
        if (try_read_line_startswith(file, "comment ")) {
            continue;
        }
        if (try_read_line_startswith(file, "element ")) {
            char element_name[MAX_PLY_ELEMENT_NAME_LENGTH];
            unsigned int element_count;
            if (sscanf(_line_buf, "element %s %u", element_name, &element_count) == EOF) STAT_ERROR("Bad element arguments");
            if (ply_stats->num_elements >= MAX_PLY_ELEMENTS) STAT_ERROR("Too many elements");
            getting_properties = true;
            
            cur_element = &ply_stats->elements[ply_stats->num_elements];

            strncpy(cur_element->name, element_name, MAX_PLY_ELEMENT_NAME_LENGTH);
            cur_element->count = element_count;
            cur_element->num_properties = 0;

            getting_properties = true;
            ply_stats->num_elements ++;
        }
        if (try_read_line_startswith(file, "property ")) {
            if (!getting_properties) STAT_ERROR("Attempting to get a property when not getting properties");
            if (cur_element->num_properties == MAX_PLY_PROPERTIES) STAT_ERROR("Too many properties");
            cur_property = &cur_element->properties[cur_element->num_properties];

            char property_type[64];
            char property_name[MAX_PLY_PROPERTY_NAME_LENGTH];

            if (sscanf(_line_buf, "property %s ", property_type) == EOF) STAT_ERROR("Bad property type");
            if (strcmp(property_type, "list") == 0) {
                cur_property->is_list = true;
                char property_list_count_type[64];
                char property_list_type[64];
                if (sscanf(_line_buf, "property %s %s %s %s",
                                               property_type,
                                    property_list_count_type,
                                          property_list_type,
                                               property_name) == EOF) STAT_ERROR("Bad list property arguments");
                cur_property->type = match_ply_type(property_list_type); // when the property is a list, this entry stores the type of the list entries
                if (cur_property->type == NULL_PLY_TYPE) STAT_ERROR("Bad list entry type");
                cur_property->list_count_type = match_ply_type(property_list_count_type); // avoid matching float types?
                if (cur_property->list_count_type == NULL_PLY_TYPE) STAT_ERROR("Bad list count type");
            } else {
                cur_property->is_list = false;
                if (sscanf(_line_buf, "property %s %s",
                                         property_type,
                                         property_name) == EOF) STAT_ERROR("Bad property arguments");
                cur_property->type = match_ply_type(property_type);
                if (cur_property->type == NULL_PLY_TYPE) STAT_ERROR("Bad property type");
            }
            strncpy(cur_property->name, property_name, MAX_PLY_PROPERTY_NAME_LENGTH);
            cur_element->num_properties ++;
        }
        /* print_ply_stats(ply_stats); */
        /* STAT_ERROR("misc. stat error"); */
    }
    return true;
}
#undef STAT_ERROR


PLYElement *ply_get_element(PLYStats *ply_stats, char *element_name)
{
    int i;
    for (i = 0; i < ply_stats->num_elements; i++) {
        if (strncpy(ply_stats->elements[i].name, element_name, MAX_PLY_ELEMENT_NAME_LENGTH) == 0) {
            break;
        }
    }
    if (i > ply_stats->num_elements) {
        return NULL;
        /* fprintf(stderr, ERROR_ALERT "Attempted to extract element \"%s\" found in file PLY stats.\n", element_name); */
        /* exit(EXIT_FAILURE); */
    }
    return &ply_stats->elements[i - 1];
}

bool ply_read_element(FILE *file, PLYElement *element, void **element_data)
{
    const int max_list_size = 8; // a waste! Do variable list lengths in elements really complicate things?
    // Dynamically allocates memory for the element data. Remember to free!
    
    printf("Extracting element:\n");
    print_ply_element(element);

    // Make space to store element data.
    int total_element_size = 0;
    for (int i = 0; i < element->num_properties; i++) {
        if (element->properties[i].is_list) {
            total_element_size += _ply_type_sizes[element->properties[i].list_count_type];
            total_element_size += max_list_size * _ply_type_sizes[element->properties[i].type];
        } else {
            total_element_size += _ply_type_sizes[element->properties[i].type];
        }
    }
    *element_data = (void *) malloc(element->count * total_element_size);

    for (int 

    return true;
}

void print_ply_element(PLYElement *element)
{
    printf("\tname: %s\n", element->name);
    printf("\tcount: %d\n", element->count);
    printf("\tnum_properties: %d\n", element->num_properties);
    for (int i = 0; i < element->num_properties; i++) {
        PLYProperty *property = &element->properties[i];
        printf("\tProperty %d:\n", i);
        printf("\t\tproperty name: %s\n", property->name);
        if (property->is_list) {
            printf("\t\tis_list: true\n");
            printf("\t\tentry type: %d (%s)\n", property->type, _ply_type_names[property->type]);
            printf("\t\tlist_count_type: %d (%s)\n", property->list_count_type, _ply_type_names[property->list_count_type]);
            printf("\t\tproperty type: %d (%s)\n", property->type, _ply_type_names[property->type]);
        } else {
            printf("\t\tis_list: false\n");
            printf("\t\tproperty type: %d (%s)\n", property->type, _ply_type_names[property->type]);
        }
    }
}


void print_ply_stats(PLYStats *ply_stats)
{
    printf("PLY stats:\n");
    printf("format: %d\n", ply_stats->format);
    printf("num_elements: %d\n", ply_stats->num_elements);
    for (int i = 0; i < ply_stats->num_elements; i++) {
        PLYElement *element = &ply_stats->elements[i];
        printf("Element %d:\n", i);
        print_ply_element(element);
    }
}



