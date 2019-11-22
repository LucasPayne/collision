/*================================================================================
   Mesh and mesh generation module.
================================================================================*/
#ifndef HEADER_DEFINED_MESH
#define HEADER_DEFINED_MESH
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "matrix_mathematics.h"
#include "helper_definitions.h"
#include "helper_gl.h"

enum ShaderType {
    Vertex,
    Fragment,
    Geometry,
    TesselationControl,
    TesselationEvaluation,
    NUM_SHADER_TYPES
};

typedef struct Renderer_s {
    Matrix4x4f view_matrix;

    // Standard uniforms
    GLuint uniform_modelview_matrix;

    // Todo: how to organize this?
    // Variable list of uniforms that should update on mesh render, versus in the loop,
    // and variable types of vertex attributes? (vertex formats)

    GLuint program;

    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint geometry_shader;
    GLuint tesselation_control_shader;
    GLuint tesselation_evaluation_shader;
    char *vertex_shader_path;
    char *fragment_shader_path;
    char *geometry_shader_path;
    char *tesselation_control_shader_path;
    char *tesselation_evaluation_shader_path;
} Renderer;

#define MAX_MESH_NAME_LENGTH 32
typedef struct Mesh_s {
    /* Storage and allocation format: contiguous, vertices then triangles.
     */
    char name[MAX_MESH_NAME_LENGTH + 1];
    unsigned int num_vertices;
    float *vertices;
    unsigned int num_triangles;
    unsigned int *triangles;
} Mesh;


#define MAX_MESH_VBOS 5
typedef struct MeshHandle_s {
    char name[MAX_MESH_NAME_LENGTH + 1];
    GLuint vao;
    GLuint vbos[MAX_MESH_VBOS + 1]; // 0 terminated
    unsigned int num_vertices;
    unsigned int num_triangles;
} MeshHandle;

void free_mesh(Mesh *mesh);
void upload_and_free_mesh(MeshHandle *mesh_handle, Mesh *mesh);
void bind_renderer(Renderer *renderer);
void render_mesh(Renderer *renderer, MeshHandle *mesh_handle, Matrix4x4f *model_matrix);

void create_cube_mesh(Mesh *mesh, float size);

void make_sphere(Mesh *mesh, float radius, int tess);
void zero_init_renderer(Renderer *renderer);
void new_renderer_vertex_fragment(Renderer *renderer, char *vertex_shader_path, char *fragment_shader_path);

GLuint renderer_shader_of_type(Renderer *renderer, int shader_type);
char *renderer_shader_path_of_type(Renderer *renderer, int shader_type);
void print_renderer(Renderer *renderer);

void print_mesh_handle(MeshHandle *mesh_handle);

#endif // HEADER_DEFINED_MESH
