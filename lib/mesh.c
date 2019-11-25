/*--------------------------------------------------------------------------------
   Definitions for the mesh and mesh rendering module.
   See the header for details
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

// Static helper functions
//--------------------------------------------------------------------------------
static void zero_mesh_handle(MeshHandle *mesh_handle);
static void add_mesh_handle_vbo(MeshHandle *mesh_handle, GLuint vbo);
//--------------------------------------------------------------------------------

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

void free_mesh(Mesh *mesh)
{
    free(mesh->vertices);
    free(mesh->triangles);
}

void upload_and_free_mesh(MeshHandle *mesh_handle, Mesh *mesh)
{
    /* Upload the mesh data and fill the mesh handle structure with information (so it can be freed, drawn, etc.)
     * Frees the mesh afterward (should?)
     *
     * NOTES:
     *      Could have options for buffer hints (dynamic/static/stream draw).
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
    renderer_upload_uniforms(renderer);
}


void renderer_upload_uniforms(Renderer *renderer)
{
    /* Upload the uniform data */
    for (int i = 0; i < renderer->num_uniforms; i++) {
        if (renderer->uniforms[i].get_uniform_value == NULL) {
            // better check this when calling a pointed-to function
            fprintf(stderr, "ERROR: not initialized a uniform value-getting function for renderer.\n");
            exit(EXIT_FAILURE);
        }
        if (renderer->uniforms[i].type == GL_FLOAT) {
            float val = renderer->uniforms[i].get_uniform_value().float_value;
            glUniform1f(renderer->uniforms[i].location, val);
        }
        else if (renderer->uniforms[i].type == GL_INT) {
            int val = renderer->uniforms[i].get_uniform_value().int_value;
            glUniform1i(renderer->uniforms[i].location, val);
        }
    }
}

void render_mesh(Renderer *renderer, MeshHandle *mesh_handle, Matrix4x4f *model_matrix, Matrix4x4f *view_matrix, Matrix4x4f *projection_matrix)
{
    bind_renderer(renderer);

    // Concatenate the given model matrix with the given view matrix and projection matrix
    Matrix4x4f mvp_matrix;
    identity_matrix4x4f(&mvp_matrix);

    right_multiply_matrix4x4f(&mvp_matrix, projection_matrix);

    // Because these are actually the coordinate frames, they are inverted
    // (transposed, since they are orthonormal matrices (hopefully)).
    // @@previous_problem@@:
    //      since they are coordinate frames, they must be explicitly inverted. This reverses
    //      the order they appear in the mvp matrix calculation. Also, should just think of
    //      what the transformations actually do. This gets the matrix P M^-1 V^-1, ...
    // into model relative coordinates
    right_multiply_by_transpose_matrix4x4f(&mvp_matrix, model_matrix);
    // into view coordinates
    right_multiply_by_transpose_matrix4x4f(&mvp_matrix, view_matrix);

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

    renderer->num_uniforms = 0;
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

//--- this shouldn't have to be done for every struct ...
void serialize_mesh_handle(FILE *file, MeshHandle *mesh_handle)
{
    fprintf(file, "name: %s\n", mesh_handle->name);
    fprintf(file, "vao: %d\n", mesh_handle->vao);
    fprintf(file, "vbos: ["); //--- need standards for serialization
    for (int i = 0; i < MAX_MESH_VBOS; i++) {
        if (mesh_handle->vbos[i] == 0) break;
        if (i != 0) fprintf(file, ", ");
        fprintf(file, "%d", mesh_handle->vbos[i]);
    }
    fprintf(file, "]\n");
    fprintf(file, "num_vertices: %u\n", mesh_handle->num_vertices);
    fprintf(file, "num_triangles: %u\n", mesh_handle->num_triangles);
}

Uniform *renderer_add_uniform(Renderer *renderer, char *name, UniformData (*get_uniform_value)(void), GLuint uniform_type)
{
    // uniform type: a GL_ type
    if (renderer->num_uniforms >= MAX_RENDERER_UNIFORMS) {
        fprintf(stderr, "ERROR: too many uniforms added to renderer.\n");
        exit(EXIT_FAILURE);
    }
    Uniform *new_uniform = &renderer->uniforms[renderer->num_uniforms];
    
    if (strlen(name) > MAX_UNIFORM_NAME_LENGTH) {
        fprintf(stderr, "ERROR: uniform name \"%s\" too long (MAX_UNIFORM_NAME_LENGTH: %d).\n", name, MAX_UNIFORM_NAME_LENGTH);
        exit(EXIT_FAILURE);
    }
    strncpy(new_uniform->name, name, MAX_UNIFORM_NAME_LENGTH);

    new_uniform->location = glGetUniformLocation(renderer->program, new_uniform->name);
    new_uniform->get_uniform_value = get_uniform_value;
    new_uniform->type = uniform_type;

    renderer->num_uniforms ++;
    return new_uniform;
}

void renderer_recompile_shaders(Renderer *renderer)
{
    if (renderer->vertex_shader != 0) {
        load_and_compile_shader(renderer->vertex_shader, renderer->vertex_shader_path);
    }
    if (renderer->fragment_shader != 0) {
        load_and_compile_shader(renderer->fragment_shader, renderer->fragment_shader_path);
    }
    // ----
    // ... and other types ...
    link_shader_program(renderer->program);
}
