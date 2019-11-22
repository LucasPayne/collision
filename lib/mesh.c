/*--------------------------------------------------------------------------------
   Definitions for the meshes and mesh generation module.
--------------------------------------------------------------------------------*/

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "matrix_mathematics.h"
#include "helper_definitions.h"
#include "mesh.h"

static void zero_mesh_handle(MeshHandle *mesh_handle);
static void add_mesh_handle_vbo(MeshHandle *mesh_handle, GLuint vbo);

void create_cube_mesh(Mesh *mesh, float size)
{
    strncpy(mesh->name, "Cube", MAX_MESH_NAME_LENGTH);
    mesh->num_vertices = 8;
    mesh->num_triangles = 12;

    mesh->vertices = (float *) malloc(3 * 8 * sizeof(float));
    for (int i = 0; i < 8; i++) {
        mesh->vertices[3*i + 0] = ((i >> 0) & 0x1) * size - size/2.0;
        mesh->vertices[3*i + 1] = ((i >> 1) & 0x1) * size - size/2.0;
        mesh->vertices[3*i + 2] = ((i >> 2) & 0x1) * size - size/2.0;
    }
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
}

void make_sphere(Mesh *mesh, float radius, int tess)
{
    // bottom vertex, tesselated vertices tess around and tess in between bottom and top, and the top vertex
    int num_vertices = 1 + tess*tess + 1;
    float *vertices = (float *) malloc(sizeof(float) * 3*num_vertices);
    mem_check(vertices);
    // triangle fans from bottom and top, and tess strips of tess vertices create 2*(tess-1)*tess triangles
    // ???
    int num_triangles = tess + 2*tess*tess + tess;
    unsigned int *triangle_indices = (unsigned int *) malloc(sizeof(int) * 3*num_triangles);
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
#if 1
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
#if 1
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
#endif
        }
    }
#endif

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

    strncpy(mesh->name, "Sphere", MAX_MESH_NAME_LENGTH);
    mesh->num_vertices = num_vertices;
    mesh->num_triangles = num_triangles;
    mesh->vertices = vertices;
    mesh->triangles = triangle_indices;
}

void free_mesh(Mesh *mesh)
{
    free(mesh->vertices);
    free(mesh->triangles);
}

static void zero_mesh_handle(MeshHandle *mesh_handle)
{
    mesh_handle->vao = 0;
    memset(mesh_handle->vbos, 0, sizeof(mesh_handle->vbos));
    mesh_handle->num_vertices = 0;
    mesh_handle->num_triangles = 0;
}

static void add_mesh_handle_vbo(MeshHandle *mesh_handle, GLuint vbo)
{
    for (int i = 0; i < MAX_MESH_VBOS; i++) {
        if (mesh_handle->vbos[i] == 0) {
            mesh_handle->vbos[i] = vbo;
            return;
        }
    }
    fprintf(stderr, "ERROR: too many vbos for mesh %s. The maximum number of vbos for a mesh handle is %d.\n",
                    mesh_handle->name, MAX_MESH_VBOS);
    exit(EXIT_FAILURE);
}

void upload_and_free_mesh(MeshHandle *mesh_handle, Mesh *mesh)
{
    /* Upload the mesh data and fill the mesh handle structure with information (so it can be freed, drawn, etc.)
     * Could have options for buffer hints (dynamic/static/stream draw).
     *
     * Frees the mesh afterward (should?)
     */
    zero_mesh_handle(mesh_handle); // make sure it is zero initialized before filling
    // copy over basic attributes
    strncpy(mesh_handle->name, mesh->name, MAX_MESH_NAME_LENGTH);
    mesh_handle->num_triangles = mesh->num_triangles;
    mesh_handle->num_vertices = mesh->num_vertices;
    const GLuint attribute_location_vPosition = 0;

    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, 3*sizeof(float) * mesh->num_vertices, mesh->vertices, GL_DYNAMIC_DRAW);

    GLuint triangle_buffer;
    glGenBuffers(1, &triangle_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangle_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3*sizeof(unsigned int) * mesh->num_triangles, mesh->triangles, GL_DYNAMIC_DRAW);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangle_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glVertexAttribPointer(attribute_location_vPosition, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    glEnableVertexAttribArray(attribute_location_vPosition);

    mesh_handle->vao = vao;
    add_mesh_handle_vbo(mesh_handle, vertex_buffer);
    add_mesh_handle_vbo(mesh_handle, triangle_buffer);

    free_mesh(mesh);
}


void bind_renderer(Renderer *renderer)
{
    /* Binding the renderer loads the program linked from the shaders.
     */
    glUseProgram(renderer->program);
}

void render_mesh(Renderer *renderer, MeshHandle *mesh_handle, Matrix4x4f *model_matrix, Matrix4x4f *view_matrix, Matrix4x4f *projection_matrix)
{
    bind_renderer(renderer);

    // Concatenate the given model matrix with the given view matrix and projection matrix
    Matrix4x4f mvp_matrix;
    identity_matrix4x4f(&mvp_matrix);

    right_multiply_matrix4x4f(&mvp_matrix, projection_matrix);
    // into view coordinates
    right_multiply_by_transpose_matrix4x4f(&mvp_matrix, view_matrix);
    // into model relative coordinates
    right_multiply_by_transpose_matrix4x4f(&mvp_matrix, model_matrix);

    glUniformMatrix4fv(renderer->uniform_mvp_matrix, 1, GL_TRUE, (const GLfloat *) &mvp_matrix.vals);
    glBindVertexArray(mesh_handle->vao);
    glDrawElements(renderer->primitive_mode, 3 * mesh_handle->num_triangles, GL_UNSIGNED_INT, (void *) 0);
}

void zero_init_renderer(Renderer *renderer)
{
    memset(renderer, 0, sizeof(Renderer));
    // if something is "zero initialized" to something other than 0, change this here.
}

#define TRACING 1
void new_renderer_vertex_fragment(Renderer *renderer, char *vertex_shader_path, char *fragment_shader_path, GLuint primitive_mode)
{
    /* Creates a new renderer only using vertex and fragment shaders. */
    zero_init_renderer(renderer);
#if TRACING
    printf("Creating new renderer with\n");
    printf("\tVertex shader path: %s\n", vertex_shader_path);
    printf("\tFragment shader path: %s\n", fragment_shader_path);
#endif
    renderer->vertex_shader_path = (char *) malloc((strlen(vertex_shader_path) + 1) * sizeof(char));
    mem_check(renderer->vertex_shader_path);
    renderer->fragment_shader_path = (char *) malloc((strlen(fragment_shader_path) + 1) * sizeof(char));
    mem_check(renderer->fragment_shader_path);
#if TRACING
    printf("Allocated memory for shader paths.\n");
#endif

    // Copy strings over, maybe should use strncpy, but the lengths were already used to allocate memory.
    strcpy(renderer->vertex_shader_path, vertex_shader_path);
    strcpy(renderer->fragment_shader_path, fragment_shader_path);
#if TRACING
    printf("Copied shader paths to the new renderer.\n");
#endif

    renderer->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    load_and_compile_shader(renderer->vertex_shader, renderer->vertex_shader_path);

    renderer->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    load_and_compile_shader(renderer->fragment_shader, renderer->fragment_shader_path);

    renderer->program = glCreateProgram();
    glAttachShader(renderer->program, renderer->vertex_shader);
    glAttachShader(renderer->program, renderer->fragment_shader);
    link_shader_program(renderer->program);

    renderer->primitive_mode = primitive_mode;
}


/* Helper functions to access shaders in a renderer iteratively (e.g. to print information) */
GLuint renderer_shader_of_type(Renderer *renderer, int shader_type)
{
    switch (shader_type) {
        case Vertex: return renderer->vertex_shader;
        case Fragment: return renderer->fragment_shader;
        case Geometry: return renderer->geometry_shader;
        case TesselationControl: return renderer->tesselation_control_shader;
        case TesselationEvaluation: return renderer->tesselation_evaluation_shader;
    }
    return 0;
}
char *renderer_shader_path_of_type(Renderer *renderer, int shader_type)
{
    switch (shader_type) {
        case Vertex: return renderer->vertex_shader_path;
        case Fragment: return renderer->fragment_shader_path;
        case Geometry: return renderer->geometry_shader_path;
        case TesselationControl: return renderer->tesselation_control_shader_path;
        case TesselationEvaluation: return renderer->tesselation_evaluation_shader_path;
    }
    return NULL;
}

void print_renderer(Renderer *renderer)
{
    printf("Renderer:\n");
    printf("Program id: %d\n", renderer->program);
    for (int i = 0; i < NUM_SHADER_TYPES; i++) {
        if (renderer_shader_of_type(renderer, i) != 0) {
            printf("%s\n", renderer_shader_path_of_type(renderer, i));
            printf("\tid: %d\n", renderer_shader_of_type(renderer, i));
        }
    }
}

void print_mesh_handle(MeshHandle *mesh_handle)
{
    printf("Mesh handle:\n");
    printf("Name: %s\n", mesh_handle->name);
    printf("Vertex array object: %d\n", mesh_handle->vao);
    printf("Number of vertices: %d\n", mesh_handle->num_vertices);
    printf("Number of triangles: %d\n", mesh_handle->num_triangles);
    printf("Attached VBOs:\n");
    for (int i = 0; mesh_handle->vbos[i] != 0; i++) {
        printf("\t%d\n", mesh_handle->vbos[i]);
    }
}
