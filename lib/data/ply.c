/*--------------------------------------------------------------------------------
    Definitions for the PLY file format module.
--------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
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

#define STAT_ERROR(MSG)\
{\
    printf("STAT ERROR: \n");\
    printf((  MSG ));\
    printf("\n");\
    fclose(file);\
    return false;\
}

bool ply_stat(char *filename, PLYStats *ply_stats)
// Log this somewhere, not stderr?
{
    memset(ply_stats, 0, sizeof(PLYStats)); // ?

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, ERROR_ALERT "Unable to read file \"%s\" when attempting to stat a PLY file.\n", filename);
        exit(EXIT_FAILURE);
    }
    if (!try_read_line(file, "ply")) {
        STAT_ERROR("Bad magic number");
    }
    if (try_read_line(file, "format ascii 1.0")) {
        ply_stats->format = PLY_FORMAT_ASCII_1;
    }
    else if (try_read_line(file, "format binary 1.0")) {
        ply_stats->format = PLY_FORMAT_BINARY_1;
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

            char property_type[20];
            char property_name[MAX_PLY_PROPERTY_NAME_LENGTH];
            if (sscanf(_line_buf, "property %20s %8s", property_type, property_name) == EOF) STAT_ERROR("Bad property arguments");
            if (strcmp(property_type, "float") == 0) {
                cur_property->type = PLY_FLOAT;
            } else if (strcmp(property_type, "int") == 0) {
                cur_property->type = PLY_INT;
            } else {
                STAT_ERROR("Bad property type");
            }
            strncpy(cur_property->name, property_name, MAX_PLY_PROPERTY_NAME_LENGTH);

            cur_element->num_properties ++;
        }
        print_ply_stats(ply_stats);
        STAT_ERROR("misc. stat error");
    }
    fclose(file);
    return true;
}
#undef STAT_ERROR


void print_ply_stats(PLYStats *ply_stats)
{
    printf("PLY stats:\n");
    printf("format: %d\n", ply_stats->format);
    printf("num_elements: %d\n", ply_stats->num_elements);
    for (int i = 0; i < ply_stats->num_elements; i++) {
        PLYElement *element = &ply_stats->elements[i];
        printf("Element %d:\n", i);
        printf("name: %s\n", element->name);
        printf("count: %d\n", element->count);
        printf("num_properties: %d\n", element->num_properties);
        for (int ii = 0; ii < element->num_properties; ii++) {
            PLYProperty *property = &element->properties[ii];
            printf("Property %d:\n", ii);
            printf("property name: %s\n", property->name);
            printf("property type: %d\n", property->type);
        }
    }
}


