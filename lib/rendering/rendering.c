/*--------------------------------------------------------------------------------
   Rendering module.
Dependencies:
	resources:  Basic graphics objects are formed as resources, special kinds of shared objects tied to assets.
	dictionary: A basic key-value-pair file format used here for configuring graphics objects.
--------ply:        Really should not be a dependency (?).
--------------------------------------------------------------------------------*/
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include "helper_definitions.h"
#include "resources.h"
#include "ply.h" //----------Remove this dependency.
#include "rendering.h"
#include "dictionary.h"

//----only here while the querying/parsing stuff for matrices is here
#include "matrix_mathematics.h"

/*================================================================================
  Text-file configuration and resources integration.
================================================================================*/
void print_vertex_format(VertexFormat vertex_format)
{
    printf("vertex_format: ");
    for (int i = 0; i < ATTRIBUTE_BITMASK_SIZE; i++) {
        printf("%d", (vertex_format & (1 << i)) == 0 ? 0 : 1);
    }
    printf("\n");
}
VertexFormat string_to_VertexFormat(char *string)
{
    VertexFormat vertex_format = 0;
#define casemap(CHAR,VF) case ( CHAR ):\
        vertex_format |= ( VF ); break;
    puts(string);
    for (int i = 0; string[i] != '\0'; i++) {
        switch (string[i]) {
            casemap('3', VERTEX_FORMAT_3);
            casemap('C', VERTEX_FORMAT_C);
            casemap('N', VERTEX_FORMAT_N);
            casemap('U', VERTEX_FORMAT_U);
            casemap('I', (1 << ATTRIBUTE_TYPE_INDEX));
            default:
                return VERTEX_FORMAT_NONE;
        }
    }
    return vertex_format;
#undef casemap
}


static bool query_val_int(char *string, void *data)
{
    int i;
    if (sscanf(string, "%d", &i) == EOF) return false;
    memcpy(data, &i, sizeof(int));
    return true;
}
static bool query_val_bool(char *string, void *data)
{
    bool b;
    if (strcmp(string, "true") == 0) b = true;
    else if (strcmp(string, "false") == 0) b = false;
    else return false;
    memcpy(data, &b, sizeof(bool));
    return true;
}
static bool query_val_float(char *string, void *data)
{
    float f;
    if (sscanf(string, "%f", &f) == EOF) return false;
    memcpy(data, &f, sizeof(float));
    return true;
}
static bool query_val_unsigned_int(char *string, void *data)
{
    // move to dictionary's basic types (built in to querier)
    unsigned int ui;
    if (sscanf(string, "%u", &ui) == EOF) return false;
    memcpy(data, &ui, sizeof(unsigned int));
    return true;
}
static bool query_val_VertexFormat(char *string, void *data)
{
    VertexFormat vf = string_to_VertexFormat(string);
    if (vf == VERTEX_FORMAT_NONE) return false;
    memcpy(data, &vf, sizeof(vf));
    return true;
}
static bool query_val_vec4(char *string, void *data)
{
    // format: 0, 1, 2, 3
    // [0]
    // [1]
    // [2]
    // [3]
    char *p = string;
    vec4 v;
    int chars_read;
    for (int i = 0; i < 4; i++) {
        if (sscanf(p, (i < 3) ? "%f%n," : "%f%n", &v.vals[i], &chars_read) == EOF) return false;
        p += chars_read + 1;
    }
    memcpy(data, &v, sizeof(vec4));
    return true;
}
static bool query_val_mat4x4(char *string, void *data)
{
    // format: 0 1 2 3 ; 4 5 6 7 ; 8 9 10 11 ; 12 13 14 15
    // [ 0 4 8 12  ]
    // [ 1 5 9 13  ]
    // [ 2 6 10 14 ]
    // [ 3 7 11 15 ]
    Matrix4x4f matrix;
    char *p = string;
#define eat() { while (isspace(*p)) { p++; } }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            eat();
            int num_chars;
            if (sscanf(p, "%f%n", &matrix.vals[4*i + j], &num_chars) == EOF) {
                return false;
            }
            p += num_chars;
            eat();
            if (j < 3 && *p++ != ',') {
                return false;
            } else if (i < 3 && j == 3 && *p++ != ';') {
                return false;
            }
        }
    }
    memcpy(data, &matrix, sizeof(Matrix4x4f));
    return true;
}
void dict_query_rules_rendering(DictQuerier *q)
{
    dict_query_rule_add(q, "VertexFormat", query_val_VertexFormat);
    
    // move to dictionary's basic types 
    dict_query_rule_add(q, "int", query_val_int);
    dict_query_rule_add(q, "unsigned int", query_val_unsigned_int);
    dict_query_rule_add(q, "float", query_val_float);
    dict_query_rule_add(q, "bool", query_val_bool);

    // move to matrix mathematics' types
    dict_query_rule_add(q, "mat4x4", query_val_mat4x4);
    dict_query_rule_add(q, "vec4", query_val_vec4);
}

void init_resources_rendering(void)
{
    add_resource_type_no_unload(Shader);
    add_resource_type_no_unload(Geometry);
    add_resource_type_no_unload(Texture);

    add_resource_type_no_unload(MaterialType);
    add_resource_type_no_unload(Material);
}

