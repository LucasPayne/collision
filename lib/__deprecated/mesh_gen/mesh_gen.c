/*--------------------------------------------------------------------------------
   Definitions for the mesh generation module.
   See the header for details.
--------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mesh.h"
#include "mesh_gen.h"

void create_cube_mesh(Mesh *mesh, float size)
{
    mesh->vertex_format = VERTEX_FORMAT_3C;
    mesh->num_vertices = 8;
    mesh->num_triangles = 12;

    mesh->attribute_data[ATTRIBUTE_TYPE_POSITION] = (float *) malloc(3 * 8 * sizeof(float));
    mem_check(mesh->attribute_data[ATTRIBUTE_TYPE_POSITION]);
    float *vertices = (float *) mesh->attribute_data[ATTRIBUTE_TYPE_POSITION];

    mesh->attribute_data[ATTRIBUTE_TYPE_COLOR] = (float *) malloc(3 * 8 * sizeof(float));
    mem_check(mesh->attribute_data[ATTRIBUTE_TYPE_COLOR]);
    float *colors = (float *) mesh->attribute_data[ATTRIBUTE_TYPE_COLOR];

    for (int i = 0; i < 8; i++) {
        vertices[3*i + 0] = ((i >> 0) & 0x1) * size - size/2.0;
        vertices[3*i + 1] = ((i >> 1) & 0x1) * size - size/2.0;
        vertices[3*i + 2] = ((i >> 2) & 0x1) * size - size/2.0;
    }
    float color_data[3 * 8] = {
        0.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0,
        0.0, 1.0, 1.0,
        1.0, 1.0, 1.0,
        1.0, 0.0, 0.0,
        0.5, 1.0, 0.0,
    };
    memcpy(colors, color_data, sizeof(color_data));

    mesh->triangles = (unsigned int *) malloc(3 * 12 * sizeof(unsigned int));
    unsigned int triangles[3*12] = { 0, 1, 2,
                                     1, 3, 2,
                                     3, 1, 5,
                                     7, 3, 5,
                                     5, 6, 7,
                                     5, 4, 6,
                                     4, 0, 2,
                                     4, 2, 6,
                                     6, 2, 3,
                                     6, 3, 7,
                                     4, 5, 1,
                                     4, 1, 0
    };
    memcpy(mesh->triangles, triangles, sizeof(triangles));
#if 0 // grabbing some cube data ...
    for (int i = 0; i < mesh->num_triangles; i++) {
        printf("[%d %d %d]\n", mesh->triangles[3*i + 0], mesh->triangles[3*i + 1], mesh->triangles[3*i + 2]);
    }
    exit(EXIT_SUCCESS);
#endif
}

void make_sphere(Mesh *mesh, float radius, int tess)
{
    mesh->vertex_format = VERTEX_FORMAT_3C;

    // bottom vertex, tesselated vertices tess around and tess in between bottom and top, and the top vertex
    int num_vertices = 1 + tess*tess + 1;
    float *vertices = (float *) malloc(sizeof(float) * 3*num_vertices);
    mem_check(vertices);

    float *colors = (float *) malloc(sizeof(float) * 3*num_vertices);
    mem_check(colors);

    // triangle fans from bottom and top, and tess strips of tess vertices create 2*(tess-1)*tess triangles
    // ???
    int num_triangles = tess + 2*tess*tess + tess;
    unsigned int *triangle_indices = (unsigned int *) malloc(sizeof(unsigned int) * 3*num_triangles);
    mem_check(triangle_indices);

    // bottom vertex
    vertices[0] = 0;
    vertices[1] = -radius;
    vertices[2] = 0;
    // top vertex
    vertices[3*(num_vertices - 1) + 0] = 0;
    vertices[3*(num_vertices - 1) + 1] = radius;
    vertices[3*(num_vertices - 1) + 2] = 0;

    int bottom_fan_offset = 0;
    int strips_offset = 3 * tess;
    int top_fan_offset = 3*(num_triangles-1 - tess);

    // triangle strips
    float plane_y = -radius;
    int strip_vert_number = 0;
    for (int i = 0; i < tess; i++) {
        plane_y += (2*radius)/tess;
        for (int j = 0; j < tess; j++) {
            float dist = radius * sin(acosf(plane_y / radius));
            float x = dist * cos(j*2*M_PI/tess);
            float z = dist * sin(j*2*M_PI/tess);
            int vertex_index = 1 + strip_vert_number++;
            vertices[3*vertex_index + 0] = x;
            vertices[3*vertex_index + 1] = plane_y;
            vertices[3*vertex_index + 2] = z;

            // Color data ------------------------------------------------------------
            colors[3*vertex_index + 0] = frand();
            colors[3*vertex_index + 1] = frand();
            colors[3*vertex_index + 2] = frand();
            //------------------------------------------------------------------------

            int triangle_1_loc = 3*2*vertex_index;
            int triangle_2_loc = 3*2*vertex_index + 3;
            // First triangle
            triangle_indices[strips_offset + triangle_1_loc + 0] = vertex_index;
            triangle_indices[strips_offset + triangle_1_loc + 1] = vertex_index + tess;
            triangle_indices[strips_offset + triangle_1_loc + 2] = j == tess - 1 ? vertex_index + 1 : vertex_index + tess + 1;
            // Second triangle
            triangle_indices[strips_offset + triangle_2_loc + 0] = vertex_index;
            triangle_indices[strips_offset + triangle_2_loc + 1] = j == tess - 1 ? vertex_index + 1 : vertex_index + tess + 1;
            triangle_indices[strips_offset + triangle_2_loc + 2] = j == tess - 1 ? vertex_index + 1 - tess : vertex_index + 1;
        }
    }

    // bottom fan
    for (int i = 0; i < tess; i++) {
        triangle_indices[bottom_fan_offset + 3*i + 0] = 0;
        triangle_indices[bottom_fan_offset + 3*i + 1] = (i + 1) % tess + 1;
        triangle_indices[bottom_fan_offset + 3*i + 2] = (i + 2) % tess + 1;
    }
    // top fan
    for (int i = 0; i < tess; i++) {
        triangle_indices[top_fan_offset + 3*i + 0] = num_vertices - 1;
        triangle_indices[top_fan_offset + 3*i + 1] = num_vertices - 1 - ((i + 1) % tess + 1);
        triangle_indices[top_fan_offset + 3*i + 2] = num_vertices - 1 - ((i + 2) % tess + 1);
    }

    mesh->num_vertices = num_vertices;
    mesh->num_triangles = num_triangles;
    mesh->attribute_data[ATTRIBUTE_TYPE_POSITION] = vertices;
    mesh->attribute_data[ATTRIBUTE_TYPE_COLOR] = colors;
    mesh->triangles = triangle_indices;
}
