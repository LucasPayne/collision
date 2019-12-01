/*--------------------------------------------------------------------------------
    Definitions for the PLY file format module.
--------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "helper_definitions.h"
#include "ply.h"

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


PLYElement *ply_get_element(PLYStats *ply_stats, char *element_name)
{
    //------ do not think this is correct.
    int i;
    for (i = 0; i < ply_stats->num_elements; i++) {
        if (strncmp(ply_stats->elements[i].name, element_name, MAX_PLY_ELEMENT_NAME_LENGTH) == 0) {
            break;
        }
    }
    if (i >= ply_stats->num_elements) {
        return NULL;
        /* fprintf(stderr, ERROR_ALERT "Attempted to extract element \"%s\" found in file PLY stats.\n", element_name); */
        /* exit(EXIT_FAILURE); */
    }
    return &ply_stats->elements[i];
}

PLYProperty *ply_get_property(PLYElement *element, char *property_name)
{
    int i;
    for (i = 0; i < element->num_properties; i++) {
        if (strncmp(element->properties[i].name, property_name, MAX_PLY_PROPERTY_NAME_LENGTH) == 0) {
            break;
        }
    }
    if (i >= element->num_properties) return NULL;
    return &element->properties[i];
}

bool ply_read_element(FILE *file, PLYElement *element, void **out_element_data)
{
    /* Given a pointer to a PLYElement given by a ply_stat call, modify the passed pointer to a void pointer
     * to point to newly allocated memory storing that element data.
     *
     * 64-bit integers are not supported, and < 32-bit integers are promoted to 32 bits.
     *
     * This is stored in the form
     *      [property 1][property 2][property 3 list count, entry, entry, entry, ...][property4]
     *      [property 1][property 2][property 3 list count, entry, entry, entry, ...][property4]
     *      [property 1][property 2][property 3 list count, entry, entry, entry, ...][property4]
     *      [property 1][property 2][property 3 list count, entry, entry, entry, ...][property4]
     *                                          .
     *                                          .
     *                                          .
     */

    // go to the start
    fseek(file, 0, SEEK_SET);

    // Dynamically allocates memory for the element data. Remember to free!
    
    printf("Extracting element:\n");
    print_ply_element(element);

    // Make space to store element data. This is computed from the counts given by the ply stat,
    // and the sizes of the properties of this element. List elements are capped at a constant maximum
    // number of entries.
    // ---- not checking this currently
    int total_element_size = 0;
    for (int i = 0; i < element->num_properties; i++) {
        if (element->properties[i].is_list) {
            total_element_size += _ply_type_sizes[element->properties[i].list_count_type];
            total_element_size += MAX_PLY_LIST_SIZE * _ply_type_sizes[element->properties[i].type];
        } else {
            total_element_size += _ply_type_sizes[element->properties[i].type];
        }
    }
    void *element_data = (void *) malloc(element->count * total_element_size);

    // Read the element data into this space.
    // skip past the lines [...]
    const int line_buffer_size = 4096;
    char line_buffer[line_buffer_size];
    for (int i = 0; i < element->line_start; i++) {
        fgets(line_buffer, line_buffer_size, file);
    }
    void *pos = element_data;
    for (int i = 0; i < element->count; i++) {
        // for each line of properties
        for (int k = 0; k < element->num_properties; k++) {
            // for each property
            PLYProperty *property = &element->properties[k];
            if (property->is_list) {
                // list properties
                uint32_t num_to_read; // only accept 32-bit unsigned int compatible list lengths
                if (fscanf(file, "%u", &num_to_read) != 1) { // can assume is int
                    fprintf(stderr, ERROR_ALERT "PLY file has invalid count for list property at line %d.\n", element->line_start + i);
                    exit(EXIT_FAILURE);
                }
                memcpy(element_data + total_element_size*i, &num_to_read, sizeof(uint32_t));
                pos += sizeof(uint32_t);
                // reading a literal and making sure its type is correct for this list property.
                if (property->type == PLY_DOUBLE) {
                    for (int num = 0; num < num_to_read; num++) {
                        double double_entry; // double list
                        if ((fscanf(file, "%lf", &double_entry)) != 1) {
                            fprintf(stderr, ERROR_ALERT "PLY file has non-double/invalid entry in list of doubles.\n");
                            exit(EXIT_FAILURE);
                        }
                        memcpy(pos, &double_entry, sizeof(double));
                        pos += sizeof(double);
                    }
                } else if (property->type == PLY_FLOAT) {
                    for (int num = 0; num < num_to_read; num++) {
                        float float_entry; // float list
                        if ((fscanf(file, "%f", &float_entry)) != 1) {
                            fprintf(stderr, ERROR_ALERT "PLY file has non-float/invalid entry in list of floats.\n");
                            exit(EXIT_FAILURE);
                        }
                        memcpy(pos, &float_entry, sizeof(float));
                        pos += sizeof(float);
                    }
                } else if (ply_int_type(property->type)) {
                    for (int num = 0; num < num_to_read; num++) {
                        int64_t signed_entry; // int list (arbitrary size, just copied over to the correct size.)
                            // should check size (char range, etc.)
                        if ((fscanf(file, "%ld", &signed_entry)) != 1) {
                            fprintf(stderr, ERROR_ALERT "PLY file has non-signed-int/invalid entry in list of signed ints.\n");
                            exit(EXIT_FAILURE);
                        }
                        memcpy(pos, &signed_entry, sizeof(int));
                        pos += sizeof(int);
                    }
                } else if (ply_uint_type(property->type)) {
                    for (int num = 0; num < num_to_read; num++) {
                        uint64_t unsigned_entry; // unsigned int list
                        if ((fscanf(file, "%lu", &unsigned_entry)) != 1) {
                            fprintf(stderr, ERROR_ALERT "PLY file has non-unsigned-int/invalid entry in list of unsigned ints.\n");
                            exit(EXIT_FAILURE);
                        }
                        memcpy(pos, &unsigned_entry, sizeof(unsigned int));
                        pos += sizeof(unsigned int);
                    }
                }
            } else {
                // regular properties
                if (property->type == PLY_DOUBLE) {
                    double double_entry;
                    if (fscanf(file, "%lf", &double_entry) != 1) {
                        fprintf(stderr, ERROR_ALERT "Invalid double entry in PLY file.");
                        exit(EXIT_FAILURE);
                    }
                    memcpy(pos, &double_entry, sizeof(double));
                    pos += sizeof(double);
                } else if (property->type == PLY_FLOAT) {
                    float float_entry;
                    if ((fscanf(file, "%f", &float_entry)) != 1) {
                        fprintf(stderr, ERROR_ALERT "Invalid float entry in PLY file.");
                        exit(EXIT_FAILURE);
                    }
                    memcpy(pos, &float_entry, sizeof(float));
                    pos += sizeof(float);
                } else if (ply_int_type(property->type)) {
                    int32_t signed_entry;
                    if ((fscanf(file, "%d", &signed_entry)) != 1) {
                        fprintf(stderr, ERROR_ALERT "Invalid signed int entry in PLY file.");
                        exit(EXIT_FAILURE);
                    }
                    memcpy(pos, &signed_entry, sizeof(int));
                    pos += sizeof(int);
                } else if (ply_uint_type(property->type)) {
                    uint32_t unsigned_entry; // unsigned int list
                    if ((fscanf(file, "%u", &unsigned_entry)) != 1) {
                        fprintf(stderr, ERROR_ALERT "Invalid unsigned int entry in PLY file.");
                        exit(EXIT_FAILURE);
                    }
                    memcpy(pos, &unsigned_entry, sizeof(unsigned int));
                    pos += sizeof(unsigned int);
                }
            }
        }
        /* fscanf( */
        /* fgets(line_buffer, line_buffer_size, file); */
        /* printf("%d: %s", i, line_buffer); */
    }
    *out_element_data = element_data;
    return true;
}

void print_ply_element(PLYElement *element)
{
    printf("\tname: %s\n", element->name);
    printf("\tline start: %d\n", element->line_start);
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

