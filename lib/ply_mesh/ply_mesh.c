/*--------------------------------------------------------------------------------
   PLY mesh loading, integration with the mesh module.
--------------------------------------------------------------------------------*/
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "helper_definitions.h"
#include "mesh.h"
#include "ply.h"

void load_mesh_ply(Mesh *mesh, VertexFormat vertex_format, char *ply_filename)
{
    memset(mesh, 0, sizeof(Mesh));
    printf("Loading mesh from PLY file %s ...\n", ply_filename);

    PLY *ply = read_ply(ply_filename);
    if ((vertex_format & VERTEX_FORMAT_3) != 0) {
        printf("Loading position data ...\n");
        char *pos_query = "[VERTEX|VERTICES|vertex|vertices|position|pos|positions|point|points]: \
float X|x|xpos|x_position|posx|position_x|x_coord|coord_x, \
float Y|y|ypos|y_position|posy|position_y|y_coord|coord_y, \
float Z|z|zpos|z_position|posz|position_z|z_coord|coord_z";
        float *pos_data = (float *) ply_get(ply, pos_query);
        mesh->attribute_data[ATTRIBUTE_TYPE_POSITION] = (void *) pos_data;
    }
    if ((vertex_format & VERTEX_FORMAT_C) != 0) {
        printf("Loading color data ...\n");
        char *color_query = "[COLOR|COLORS|COLOUR|COLOURS|color|colors|colour|colours]: \
float r|red|R|RED, \
float g|green|G|GREEN, \
float b|blue|B|BLUE";
        float *color_data = (float *) ply_get(ply, color_query);
        mesh->attribute_data[ATTRIBUTE_TYPE_COLOR] = (void *) color_data;
    }
    if ((vertex_format & ~VERTEX_FORMAT_3 & ~VERTEX_FORMAT_C) != 0) {
        fprintf(stderr, ERROR_ALERT "Attempted to extract mesh data with unsupported vertex format from PLY file.\n");
        exit(EXIT_FAILURE);
    }
    printf("================================================================================\n");
    printf("Mesh loaded.\n");
    printf("================================================================================\n");
}

