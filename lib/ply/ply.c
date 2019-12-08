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
static char _ply_type_names[NUM_PLY_TYPES][12] = { // do not shuffle these!
    "none_type",
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

// Internal application data for ply files is only stored as types
//      float
//      uint32_t
//      int32_t
// The ASCII is just interpreted as these, and the binary types are promoted if needed.
//      ---- demoting doubles
static bool ply_float_type(PLYType type)
{
    return type == PLY_FLOAT || type == PLY_DOUBLE;
}
static bool ply_signed_int_type(PLYType type)
{
    return type == PLY_CHAR || type == PLY_SHORT || type == PLY_INT;
}
static bool ply_unsigned_int_type(PLYType type)
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
    return PLY_NONE_TYPE;
}

//--------------------------------------------------------------------------------
// Data extraction
//--------------------------------------------------------------------------------
#define GROW_DATA(AMOUNT)\
{\
    size_t to_size = offset + ( AMOUNT );\
    if (to_size >= data_size) {\
        while (to_size >= data_size) data_size *= 2;\
        data = realloc(data, data_size);\
        mem_check(data);\
    }\
}
void ply_get_binary_data(FILE *file, PLY *ply)
{
    fseek(file, 0, SEEK_SET);
    printf("Getting PLY binary data ...\n");
    
    // read lines until the end of header (need a better way to do this for arbitrary line lengths)
    // --- line endings
    char line_buffer[4096];
    while(fgets(line_buffer, 4096, file) != NULL) {
        if (strcmp(line_buffer, "end_header\n") == 0) break;
    }
    printf("Eaten past header\n");
    
    size_t data_size = 4096;
    void *data = malloc(data_size);
    mem_check(data);
    size_t offset = 0;
    PLYElement *cur_element = ply->first_element;

    printf("Allocated some space for the read data (it can grow later).\n");

    switch(ply->format) {
    case PLY_FORMAT_ASCII_1:
        printf("Reading binary data from ASCII formatted PLY file.\n");
        while (cur_element != NULL) {
            printf("===============================\n");
            printf("Reading a new element ...\n");
            printf("===============================\n");
            // Set this element region's byte offset from the start.
            cur_element->offset = offset;

            // Since PLY files can contain lists of variable length, this keeps track of the offsets of properties.
            // --- why have this tree structure in here? why not just put in the properties?
            size_t **property_offsets = (size_t **) malloc(cur_element->num_properties * sizeof(size_t *));
            mem_check(property_offsets);
            for (int i = 0; i < cur_element->num_properties; i++) {
                property_offsets[i] = (size_t *) malloc(cur_element->count * sizeof(size_t));
                mem_check(property_offsets[i]);
            }
            printf("Allocated memory for the property offsets (it can grow later).\n");

            for (int i = 0; i < cur_element->count; i++) {
                printf("-------------------------------\n");
                printf("Reading an element entry ...\n");
                printf("-------------------------------\n");
                PLYProperty *cur_property = cur_element->first_property;
                int prop_index = 0;
                while (cur_property != NULL) {

                    property_offsets[prop_index][i] = offset; //---order

                    uint32_t list_count = 1; // leave list count at 1 if it isn't a list property.
                    if (cur_property->is_list) {
                        fscanf(file, "%u", &list_count);
                        GROW_DATA(sizeof(uint32_t));
                        memcpy(data + offset, &list_count, sizeof(uint32_t));
                        offset += sizeof(uint32_t);
                    }
                    for (int k = 0; k < list_count; k++) {
                        // Using C's standard ASCII type formats.
                        if (ply_float_type(cur_property->type)) {
                            float float_val;
                            fscanf(file, "%f", &float_val);
                            GROW_DATA(sizeof(float));
                            memcpy(data + offset, &float_val, sizeof(float));
                            offset += sizeof(float);
                        } else if (ply_unsigned_int_type(cur_property->type)) {
                            uint32_t unsigned_int_val;
                            fscanf(file, "%u", &unsigned_int_val);
                            GROW_DATA(sizeof(uint32_t));
                            memcpy(data + offset, &unsigned_int_val, sizeof(uint32_t));
                            offset += sizeof(uint32_t);
                        } else if (ply_signed_int_type(cur_property->type)) {
                            int32_t signed_int_val;
                            fscanf(file, "%d", &signed_int_val);
                            size_t to_size = offset + sizeof(int32_t);
                            GROW_DATA(sizeof(int32_t));
                            memcpy(data + offset, &signed_int_val, sizeof(int32_t));
                            offset += sizeof(int32_t);
                        }
                        printf(" - ");
                    }
                    printf("\n");
                    cur_property = cur_property->next_property;
                    // Remember to increment!
                    prop_index ++;
                    printf("Done reading a property entry\n");
                }
            }
            // Give the element the property offsets.
            cur_element->property_offsets = property_offsets;
            cur_element = cur_element->next_element;
        }
    break;
    default:
        fprintf(stderr, ERROR_ALERT "PLY format given for extraction of binary data is not implemented.\n");
        exit(EXIT_FAILURE);
    }
    // Give this data to the PLY object.
    ply->data = data;
}
#undef GROW_DATA

//--------------------------------------------------------------------------------
// Querying
//--------------------------------------------------------------------------------
#define GROW_GOT_DATA(AMOUNT)\
{\
    size_t to_size = got_data_offset + ( AMOUNT );\
    if (to_size >= got_data_size) {\
        while(to_size >= got_data_size) got_data_size *= 2;\
        got_data = realloc(got_data, got_data_size);\
        mem_check(got_data)\
    }\
}
void *ply_get(FILE *file, PLY *ply, char *query_string, int *num_entries)
{
    // num_elements: return information here about how many elements of the queried type there are.
    // Pass NULL to ignore this.
    // ////////////////////note: this shouldn't be here?
    // NOTE----Maybe shouldn't allow multiple elements in a query. It doesn't make sense to pack them together, and just one after the other is multiple queries.
    // So, alot of that multiple-elements structure here has been pointless.
    // Since data is allocated early, free it before returning an error to handle!
    
    fseek(file, 0, SEEK_SET);
    if (ply->data == NULL) { // if it is not null, it has already been loaded.
        printf("Didn't have data, getting now ...\n");
        ply_get_binary_data(file, ply);
        if (ply->data == NULL) {
            fprintf(stderr, ERROR_ALERT "Could not retrieve binary data from PLY file when querying.\n");
        }
        printf("Got the data.\n");
    }

    printf("Querying with \"%s\" ...\n", query_string);
    printf("started ply_get\n");


    PLYQuery *query = read_ply_query(query_string);
    PLYQueryElement *cur_query_element = query->first_element;


    printf("read the ply query\n");
    print_ply_query(query);

    size_t got_data_size = 4096;
    void *got_data = malloc(got_data_size);
    mem_check(got_data);
    size_t got_data_offset = 0;

    while (cur_query_element != NULL) {
        if (cur_query_element->pattern_string == NULL) {
            fprintf(stderr, ERROR_ALERT "Pattern string not set during PLY query\n");
            exit(EXIT_FAILURE);
        }
        printf("Matching new element query \"%s\" ...\n", cur_query_element->pattern_string);

        char *p = cur_query_element->pattern_string;

        PLYElement *got_element = NULL;
        while (got_element == NULL) {
            // Splits the string by bars.
            char *end = strchr(p, '|');
            if (end != NULL) {
                *end = '\0';
            }
            printf("Checking %s ...\n", p);
            //- Operate on substring ----------------------
            got_element = ply_get_element(ply, p);
            //---------------------------------------------
            if (end == NULL) break;
            p = end + 1;
        }
        if (got_element == NULL) {
            fprintf(stderr, ERROR_ALERT "Couldn't find element in PLY query.\n");
            exit(EXIT_FAILURE);
        }
        printf("Matched element %s\n", got_element->name);

        // Collect the matched properties into this malloc'd space.
        PLYProperty **got_properties = (PLYProperty **) malloc(cur_query_element->num_properties * sizeof(PLYProperty *));
        mem_check(got_properties);

        // Got an element, match its properties.
        PLYQueryProperty *cur_query_property = cur_query_element->first_property;
        int prop_index = 0;
        while (cur_query_property != NULL) {
            printf("Matching new property query \"%s\" ...\n", cur_query_property->pattern_string);
            if (cur_query_property->pattern_string == NULL) {
                fprintf(stderr, ERROR_ALERT "Query string not set during PLY query\n");
                exit(EXIT_FAILURE);
            }
            PLYProperty *got_property = NULL;
            char *p = cur_query_property->pattern_string;
            while (got_property == NULL) {
                // Splits the string by bars.
                char *end = strchr(p, '|');
                if (end != NULL) {
                    *end = '\0';
                }
                //- Operate on substring ----------------------
                //------------------------------------------------Need to check the type!
                printf("looking for %s ...\n", p);
                got_property = ply_get_property(got_element, p);
                //---------------------------------------------
                if (end == NULL) break;
                p = end + 1;
            }
            if (got_property == NULL) {
                fprintf(stderr, ERROR_ALERT "Couldn't find property in PLY query.\n");
                exit(EXIT_FAILURE);
            }
            printf("Matched property query with \"%s\".\n", got_property->name);
            // Matched a property, add this to the gotten properties for packing later.
            got_properties[prop_index] = got_property;
            cur_query_property = cur_query_property->next;
            // Increment the property index!
            prop_index++;
        }

        // Now that an element pattern has been matched, and its property patterns have been matched to actual properties,
        // pack these.
        printf("Transfering data and packing according to format pattern ...\n");
        size_t ply_data_offset = 0;
        for (int i = 0; i < got_element->count; i++) {
            // Get this ply element data (all properties in the order of the PLY file).
            for (int k = 0; k < cur_query_element->num_properties; k++) {
                //////////////////////////////////////////////////////////////////////////////////
                // ----put the offset info in the property instead. this just gets the index, awfully.
                //-----------------------------------Seriously awful, change this.
                int prop_index = 0;
                {
                    PLYProperty *cur_property = got_element->first_property;
                    while (cur_property != NULL) {
                        if (cur_property == got_properties[k]) break;
                        prop_index ++;
                        cur_property = cur_property->next_property;
                    }
                    if (prop_index >= got_element->num_properties) { fprintf(stderr, "couldn't find in awful hack\n"); exit(EXIT_FAILURE); }
                }
                //////////////////////////////////////////////////////////////////////////////////
                ///// I don't know if using count_offset is neccessary.
                uint32_t count = 1;
                size_t count_offset = 0; // shifted up if there needs to be room for a list count
                if (got_properties[k]->is_list) {
                    // the data stored here should be a 32-bit unsigned int, and denote the number of list entries to read.
                    count = ((uint32_t *) (ply->data + got_element->property_offsets[prop_index][i]))[0];
                    /* printf("count: %u\n", count); */
                    /* getchar(); */
                    GROW_GOT_DATA(sizeof(uint32_t));
                    memcpy(got_data + got_data_offset, ply->data + got_element->property_offsets[prop_index][i], sizeof(uint32_t));
                    count_offset += sizeof(uint32_t);
                }
                size_t sz;
                // Build up the size according to the promoted (----demoted) types of the entries (either if list or singleton property).
                if (ply_float_type(got_properties[k]->type)) sz = sizeof(float) * count;
                else if (ply_unsigned_int_type(got_properties[k]->type)) sz = sizeof(uint32_t) * count;
                else if (ply_signed_int_type(got_properties[k]->type)) sz = sizeof(int32_t) * count;
                else {
                    fprintf(stderr, ERROR_ALERT "Unrecognized property type when extracting binary data from PLY file through query.\n");
                    exit(EXIT_FAILURE);
                }
                GROW_GOT_DATA(sz);
                // Store the i'th entry in the PLY data of the k'th matched property. This takes into account variable list property lengths.
                memcpy(got_data + got_data_offset + count_offset, ply->data + got_element->property_offsets[prop_index][i] + count_offset, sz);
                got_data_offset += sz + count_offset;
            }
            //////////////////////////////////////////////////////////////////////////////////
            // Here, expecting that there is only one element. Have to remove multiple-elements querying.
            if (num_entries != NULL) *num_entries = got_element->count;
        }
        printf("Finished extracting and packing element.\n");

        // Remember to free!
        free(got_properties);
        cur_query_element = cur_query_element->next;
    }

    destroy_ply_query(query);
    //---check if forgot to free anything.
    printf("Completed getting queried data.\n");
    return got_data;
}
#undef GROW_GOT_DATA

void *ply_get_element(PLY *ply, char *element_name)
{
    PLYElement *cur_element = ply->first_element;
    while (cur_element != NULL) {
        printf("\"%s\" = \"%s\"?\n", cur_element->name, element_name);
        if (cur_element->name != NULL && strcmp(cur_element->name, element_name) == 0) {
            printf("Got!\n");
            return cur_element;
        }
        cur_element = cur_element->next_element;
    }
    return NULL;
}
void *ply_get_property(PLYElement *element, char *property_name)
{
    PLYProperty *cur_property = element->first_property;
    while (cur_property != NULL) {
        printf("\"%s\" = \"%s\"?\n", cur_property->name, property_name);
        if (cur_property->name != NULL && strcmp(cur_property->name, property_name) == 0) {
            return cur_property;
        }
        cur_property = cur_property->next_property;
    }
    return NULL;
}

//--------------------------------------------------------------------------------
// Initializers and destructors
//--------------------------------------------------------------------------------
void init_ply(PLY *ply)
{
    ply->format = PLY_FORMAT_NONE;
    ply->num_elements = 0;
    ply->first_element = NULL;
}
void destroy_ply(PLY *ply)
{
    if (ply->first_element != NULL) {
        PLYElement *cur_element = ply->first_element;
        do {
            PLYElement *destroy_this = cur_element;
            cur_element = cur_element->next_element;
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
            cur_property = cur_property->next_property;
            destroy_ply_property(destroy_this);
        } while (cur_property != NULL);
    }
    if (ply_element->property_offsets != NULL) {
        for (int i = 0; i < ply_element->num_properties; i++) {
            if (ply_element->property_offsets[i] != NULL) {
                free(ply_element->property_offsets[i]);
            }
        }
    }
    free(ply_element);
}
void init_ply_property(PLYProperty *ply_property)
{
    ply_property->name = NULL;
    ply_property->type = PLY_NONE_TYPE;
    ply_property->is_list = false;
    ply_property->list_count_type = PLY_NONE_TYPE;
    ply_property->next_property = NULL;
}
void destroy_ply_property(PLYProperty *ply_property)
{
    if (ply_property->name != NULL) free(ply_property->name);
    free(ply_property);
}

void destroy_ply_query(PLYQuery *query)
{
    if (query->query_string != NULL) free(query->query_string);
    PLYQueryElement *cur_element = query->first_element;
    while (cur_element != NULL) {
        PLYQueryElement *destroy_this = cur_element;
        cur_element = cur_element->next;
        destroy_ply_query_element(destroy_this);
    }
    free(query);
}
void destroy_ply_query_element(PLYQueryElement *query_element)
{
    if (query_element->pattern_string != NULL) free(query_element->pattern_string);
    PLYQueryProperty *cur_property = query_element->first_property;
    while (cur_property != NULL) {
        PLYQueryProperty *destroy_this = cur_property;
        cur_property = cur_property->next;
        destroy_ply_query_property(destroy_this);
    }
    free(query_element);
}
void destroy_ply_query_property(PLYQueryProperty *query_property)
{
    if (query_property->pattern_string != NULL) free(query_property->pattern_string);
    free(query_property);
}

//--------------------------------------------------------------------------------
// Printers and serializers
//--------------------------------------------------------------------------------
void print_ply(PLY *ply)
{
    printf("PLY object:\n");
    printf("============================================================\n");
    printf("format: %.80s (%d)\n", _ply_format_names[ply->format], ply->format);
    printf("num_elements: %d\n", ply->num_elements);
    printf("Elements:\n");
    if (ply->first_element == NULL) {
        printf("no elements\n");
    } else {
        PLYElement *print_this = ply->first_element;
        do {
            printf("------------------------------------------------------------\n");
            print_ply_element(print_this);
            print_this = print_this->next_element;
        } while (print_this != NULL);
    }
}
void print_ply_element(PLYElement *ply_element)
{
    printf("name: %.80s\n", ply_element->name == NULL ? "!!! NO NAME GIVEN !!! (or this is the name, for some reason)" : ply_element->name);
    printf("count: %d\n", ply_element->count);
    printf("num_properties: %d\n", ply_element->num_properties);
    printf("Properties:\n");
    if (ply_element->first_property == NULL) {
        printf("no properties\n");
    } else {
        PLYProperty *print_this = ply_element->first_property;
        do {
            printf(" - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
            print_ply_property(print_this);
            print_this = print_this->next_property;
        } while (print_this != NULL);
    }
}
void print_ply_property(PLYProperty *ply_property)
{
    printf("name: %.80s\n", ply_property->name == NULL ? "!!! NO NAME GIVEN !!! (or this is the name, for some reason)" : ply_property->name);
    printf("type: %.80s (%d)\n", _ply_type_names[ply_property->type], ply_property->type);
    printf("is_list: %s\n", ply_property->is_list ? "true" : "false");
    printf("list_count_type: %.80s (%d)\n", _ply_type_names[ply_property->list_count_type], ply_property->list_count_type);
}
#define SERIALIZE_NONE(THING) "!!! NO " THING " GIVEN !!! (or this is it for some reason)"
void print_ply_query(PLYQuery *ply_query)
{
    printf("query_string: \"%.500s\"\n", ply_query->query_string == NULL ? SERIALIZE_NONE("QUERY STRING") : ply_query->query_string);
    printf("num_elements: %d\n", ply_query->num_elements);
    if (ply_query->first_element == NULL) {
        printf("no elements\n");
    } else {
        PLYQueryElement *print_this = ply_query->first_element;
        do {
            printf("------------------------------------------------------------\n");
            print_ply_query_element(print_this);
            print_this = print_this->next;
        } while (print_this != NULL);
    }
}
void print_ply_query_element(PLYQueryElement *ply_query_element)
{
    printf("pattern_string: \"%.500s\"\n", ply_query_element->pattern_string == NULL ? SERIALIZE_NONE("PATTERN STRING") : ply_query_element->pattern_string);
    printf("num_properties: %d\n", ply_query_element->num_properties);
    if (ply_query_element->first_property == NULL) {
        printf("no properties\n");
    } else {
        PLYQueryProperty *print_this = ply_query_element->first_property;
        do {
            printf(" - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
            print_ply_query_property(print_this);
            print_this = print_this->next;
        } while (print_this != NULL);
    }
}
void print_ply_query_property(PLYQueryProperty *ply_query_property)
{
    printf("pattern_string: \"%.500s\"\n", ply_query_property->pattern_string == NULL ? SERIALIZE_NONE("PATTERN STRING") : ply_query_property->pattern_string);
    printf("pack_type: %.80s (%d)\n", _ply_type_names[ply_query_property->pack_type], ply_query_property->pack_type);
}


#undef SERIALIZE_NONE
