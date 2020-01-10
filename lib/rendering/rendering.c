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
#include "ply.h"
#include "rendering.h"

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

void init_resources_rendering(void)
{
    add_resource_type_no_unload(Shader);
    add_resource_type_no_unload(Geometry);
    add_resource_type_no_unload(Texture);

    add_resource_type_no_unload(MaterialType);
    add_resource_type_no_unload(Material);
}

