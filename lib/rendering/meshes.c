/*--------------------------------------------------------------------------------
Meshes component of the rendering module.
A mesh is just a Geometry object, with triangle primitives, probably loaded from a file.
This module consists of
    - Functions to load meshes from asset files into a MeshData struct.
    - A function, upload_mesh, to upload the MeshData to vram and return a Geometry object.
      Underyling this are the usual geometry-specification calls.
--------------------------------------------------------------------------------*/
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include "helper_definitions.h"
#include "resources.h"
#include "ply.h"
#include "rendering.h"


void MeshData_calculate_normals(MeshData *mesh_data)
{
    float *normals = (float *) malloc(mesh_data->num_vertices * 3 * sizeof(float));
    mem_check(normals);

    const int max_num_adjacent_triangles = 16; // If this becomes a problem, either change this number or allow the array to grow for rare cases.
    const int triangle_array_length = max_num_adjacent_triangles + 1;
    // First entry in each slot is the number of adjacent triangles.
    uint32_t *triangle_arrays = (uint32_t *) calloc(mesh_data->num_vertices * triangle_array_length, sizeof(uint32_t));
    mem_check(triangle_arrays);

    // Build up adjacent-triangle arrays for each vertex.
    for (int i = 0; i < mesh_data->num_triangles; i++) {
        // For each vertex in the triangle, add the triangle index to that vertex's adjacent-triangle array.
        for (int j = 0; j < 3; j++) {
            uint32_t vertex_index = mesh_data->triangles[3*i + j];
            int num_triangles = triangle_arrays[triangle_array_length * vertex_index];
            if (num_triangles >= max_num_adjacent_triangles) {
                fprintf(stderr, ERROR_ALERT "Failed to calculate normals for mesh, as it has too many adjacent triangles for a single vertex. The maximum can be increased.\n");
                exit(EXIT_FAILURE);
            }
            triangle_arrays[triangle_array_length * vertex_index + num_triangles + 1] = i; // Add this triangle index to this vertex's triangle array.
            triangle_arrays[triangle_array_length * vertex_index] ++; // Up the count of triangles in this vertex's triangle array.
        }
    }
    // For each vertex, calculate the normals of the adjacent triangles, and average these, then normalize this. This is the normal calculated at this vertex.
    for (int i = 0; i < mesh_data->num_vertices; i++) {
        vec3 total = vec3_zero();
        for (int j = 0; j < triangle_arrays[triangle_array_length * i]; j++) {
            uint32_t triangle_index = triangle_arrays[triangle_array_length * i + j + 1];
            uint32_t a = mesh_data->triangles[3*triangle_index + 0];
            uint32_t b = mesh_data->triangles[3*triangle_index + 1];
            uint32_t c = mesh_data->triangles[3*triangle_index + 2];
            vec3 u = *((vec3 *) &mesh_data->attribute_data[Position][3*a]);
            vec3 v = *((vec3 *) &mesh_data->attribute_data[Position][3*b]);
            vec3 w = *((vec3 *) &mesh_data->attribute_data[Position][3*c]);
            vec3 tri_normal = vec3_normalize(vec3_cross(vec3_sub(v, u), vec3_sub(w, u)));
            total = vec3_add(total, tri_normal);
        }
        vec3 normal = vec3_mul(total, 1.0 / 3.0);
        memcpy(&normals[3*i], &normal, sizeof(vec3));
    }
    mesh_data->attribute_data[Normal] = normals;
    mesh_data->attribute_data_sizes[Normal] = mesh_data->num_vertices * 3 * sizeof(float);
    printf("Calculated normals:\n");
    for (int i = 0; i < mesh_data->num_vertices; i++) {
        printf("%.2f %.2f %.2f\n", ((float *) mesh_data->attribute_data[Normal])[3*i + 0],
                                   ((float *) mesh_data->attribute_data[Normal])[3*i + 1],
                                   ((float *) mesh_data->attribute_data[Normal])[3*i + 2]);
    }
    getchar();
}

void load_mesh_ply(MeshData *mesh, VertexFormat vertex_format, FILE *file)
{
    memset(mesh, 0, sizeof(MeshData));
    mesh->vertex_format = vertex_format;

    PLY *ply = read_ply(file);

    if ((vertex_format & VERTEX_FORMAT_3) == 0) {
        fprintf(stderr, ERROR_ALERT "Attempted to load PLY mesh with invalid vertex format (no position attribute).\n");
        exit(EXIT_FAILURE);
    }
#define BASE_VERTEX_QUERY "VERTEX|VERTICES|vertex|vertices|position|pos|positions|point|points"
    
    //--------------------------------------------------------------------------------
    // Query for positions
    //--------------------------------------------------------------------------------
    char *pos_query = "[" BASE_VERTEX_QUERY "]: \
float X|x|xpos|x_position|posx|position_x|x_coord|coord_x, \
float Y|y|ypos|y_position|posy|position_y|y_coord|coord_y, \
float Z|z|zpos|z_position|posz|position_z|z_coord|coord_z";
    int num_vertices;
    void *pos_data = ply_get(file, ply, pos_query, &num_vertices);
    mesh->attribute_data[ATTRIBUTE_TYPE_POSITION] = pos_data;
    mesh->num_vertices = num_vertices;

    //--------------------------------------------------------------------------------
    // Query for colors
    //--------------------------------------------------------------------------------
    if ((vertex_format & VERTEX_FORMAT_C) != 0) {
        char *color_query = "[" BASE_VERTEX_QUERY "|COLOR|COLORS|COLOUR|COLOURS|color|colors|colour|colours]: \
float r|red|R|RED, \
float g|green|G|GREEN, \
float b|blue|B|BLUE";
        void *color_data = ply_get(file, ply, color_query, NULL);
        mesh->attribute_data[ATTRIBUTE_TYPE_COLOR] = color_data;
    }
    
    //--------------------------------------------------------------------------------
    // Query for texture coordinates
    //--------------------------------------------------------------------------------
    if ((vertex_format & VERTEX_FORMAT_U) != 0) {
        char *texture_coord_query = "[" BASE_VERTEX_QUERY "|TEXTURES|texcoords|TEXCOORD|texture_coordinates|UV|Uv|uv|texcoord|textures]: \
float u|U|textureU|texU|tex_coord_u|tex_coord_U, \
float v|V|textureV|texV|tex_coord_v|tex_coord_V";
        void *texture_coord_data = ply_get(file, ply, texture_coord_query, NULL);
        mesh->attribute_data[ATTRIBUTE_TYPE_UV] = texture_coord_data;
    }

    //--------------------------------------------------------------------------------
    // Query for normals
    //--------------------------------------------------------------------------------
    if ((vertex_format & VERTEX_FORMAT_N) != 0) {
        char *normal_query = "[" BASE_VERTEX_QUERY "]: \
float nx|nX|NX|Nx|normal_x|Normal_x|Normal_X|xNormal|XNormal, \
float ny|nY|NY|Ny|normal_y|Normal_y|Normal_Y|yNormal|YNormal, \
float nz|nZ|NZ|Nz|normal_z|Normal_z|Normal_Z|zNormal|ZNormal";
        void *normal_data = ply_get(file, ply, normal_query, NULL);
        mesh->attribute_data[ATTRIBUTE_TYPE_NORMAL] = normal_data;
    }
    //--------------------------------------------------------------------------------

    if ((vertex_format & ~VERTEX_FORMAT_3 & ~VERTEX_FORMAT_C & ~VERTEX_FORMAT_U & ~VERTEX_FORMAT_N) != 0) {
        fprintf(stderr, ERROR_ALERT "Attempted to extract mesh data with unsupported vertex format from PLY file.\n");
        exit(EXIT_FAILURE);
    }
    // Triangles and face data. This is queried for, then it is made sure each face has 3 vertex indices,
    // then packs this data into the format used for meshes, with no counts (just ...|...|... etc.).
    char *face_query = "[face|faces|triangle|triangles|tris|tri]: \
list int vertex_index|vertex_indices|indices|triangle_indices|tri_indices|index_list|indices_list";
    printf("Loaded face data.\n");
    int num_faces;
    void *face_data = ply_get(file, ply, face_query, &num_faces);
    size_t face_data_offset = 0;
    size_t triangles_size = 128;
    void *triangles = malloc(triangles_size * sizeof(uint32_t));
    size_t triangles_offset = 0;
    for (int i = 0; i < num_faces; i++) {
        if (((uint32_t *) (face_data + face_data_offset))[0] != 3) {
            // not handling triangulation of arbitrary polygon lists
            fprintf(stderr, ERROR_ALERT "Attempted to extract face from PLY file as a triangle, it does not have 3 vertex indices.\n");
            exit(EXIT_FAILURE);
        }
        face_data_offset += sizeof(uint32_t);
        // Now extract the three entries
        for (int i = 0; i < 3; i++) {
            size_t to_size = triangles_offset + sizeof(uint32_t);
            if (to_size >= triangles_size) {
                while (to_size >= triangles_size) triangles_size += 128;
                triangles = (uint32_t *) realloc(triangles, triangles_size * sizeof(uint32_t));
                mem_check(triangles);
            }
            memcpy(triangles + triangles_offset, face_data + face_data_offset, sizeof(uint32_t));
            face_data_offset += sizeof(uint32_t);
            triangles_offset = to_size;
        }
    }

    mesh->num_triangles = num_faces;
    mesh->triangles = (uint32_t *) triangles;
    free(face_data); // this is not stored in the mesh
#undef BASE_VERTEX_QUERY
}


Geometry upload_mesh(MeshData *mesh_data)
{
    // Upload a triangle mesh (for the mesh+material pair rendering of objects).
    gm_triangles(mesh_data->vertex_format);
    
    for (int i = 0; i < NUM_ATTRIBUTE_TYPES; i++) {
        if (mesh_data->vertex_format & (1 << i)) {
            attribute_buf(i, mesh_data->attribute_data[i], mesh_data->num_vertices); // hope the buffer is the right size!
        }
    }
    gm_index_buf(mesh_data->triangles, 3*mesh_data->num_triangles);
    return gm_done();
}
