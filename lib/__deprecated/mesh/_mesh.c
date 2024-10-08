/*--------------------------------------------------------------------------------
   Definitions for the mesh and mesh rendering module.
   See the header for details
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
#include "ply.h" // should remove this dependency?

// Static helper functions
//--------------------------------------------------------------------------------
static void zero_mesh_handle(MeshHandle *mesh_handle);
//--------------------------------------------------------------------------------

// Static data
//--------------------------------------------------------------------------------
// typedef struct AttributeInfo_s {
//     AttributeType attribute_type;
//     char name[MAX_ATTRIBUTE_NAME_LENGTH + 1];
//     GLenum gl_type;
//     GLuint gl_size;
// } AttributeInfo;
static const AttributeInfo g_attribute_info[NUM_ATTRIBUTE_TYPES] = {
    { ATTRIBUTE_TYPE_POSITION, "vPosition", GL_FLOAT, 3 },
    { ATTRIBUTE_TYPE_COLOR, "vColor", GL_FLOAT, 3 },
    { ATTRIBUTE_TYPE_NORMAL, "vNormal", GL_FLOAT, 3},
};
//--------------------------------------------------------------------------------

static void zero_mesh_handle(MeshHandle *mesh_handle)
{
    memset(mesh_handle, 0, sizeof(MeshHandle)); // only because currently to "zero", everything actually is zeroed.
}

void free_mesh(Mesh *mesh)
{
    // free vertex attribute data
    for (int i = 0; i < NUM_ATTRIBUTE_TYPES; i++) {
        if (mesh->attribute_data[i] != NULL) {
            free(mesh->attribute_data[i]);
        }
    }
    free(mesh->triangles);
}

void upload_mesh(MeshHandle *mesh_handle, Mesh *mesh)
{
#define TRACING 1
#define TRACING_VERBOSE 0
    /* Upload the mesh data and fill the mesh handle structure with information (so it can be freed, drawn, etc.)
     * Frees the mesh afterward (should?)
     *
     * NOTES:
     *      Could have options for buffer hints (dynamic/static/stream draw).
     */
    // make sure the mesh handle is zero initialized before filling.
#if TRACING
    printf("Uploading mesh and associating it to a mesh handle\n");
    print_mesh(mesh);
#endif
    zero_mesh_handle(mesh_handle); 
    
    // copy over basic properties.
    mesh_handle->num_triangles = mesh->num_triangles;
    mesh_handle->num_vertices = mesh->num_vertices;
    mesh_handle->vertex_format = mesh->vertex_format;

#if TRACING_VERBOSE
    printf("Uploading vertex attribute data ...\n");
#endif
    // upload vertex attribute data and associate to the mesh handle.
    for (int i = 0; i < NUM_ATTRIBUTE_TYPES; i++) {
        if (((mesh->vertex_format >> i) & 1) == 1) { // vertex format has attribute i set
#if TRACING_VERBOSE
            printf("uploading and associating vertex attribute %s\n", g_attribute_info[i].name);
#endif
            if (mesh->attribute_data[i] == NULL) {
                fprintf(stderr, ERROR_ALERT "Attempted to upload mesh which does not have data for one of its attributes.\n");
                exit(EXIT_FAILURE);
            }
            GLuint attribute_buffer;
            glGenBuffers(1, &attribute_buffer);
            glBindBuffer(GL_ARRAY_BUFFER, attribute_buffer);
            glBufferData(GL_ARRAY_BUFFER,
                         g_attribute_info[i].gl_size * gl_type_size(g_attribute_info[i].gl_type) * mesh->num_vertices,
                         mesh->attribute_data[i],
                         GL_DYNAMIC_DRAW);
            // associate this buffer id to the mesh handle.
            mesh_handle->attribute_vbos[i] = attribute_buffer;
#if TRACING_VERBOSE
            printf("finished uploading this attribute, given vbo id %d\n", attribute_buffer);
#endif
        }
    }
    // upload triangle index data and associate to the mesh handle.
#if TRACING_VERBOSE
    printf("uploading triangle indices\n");
#endif
    GLuint triangle_buffer;
    glGenBuffers(1, &triangle_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangle_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3*sizeof(unsigned int) * mesh->num_triangles, mesh->triangles, GL_DYNAMIC_DRAW);
    // associate this triangle buffer id to the mesh handle.
    mesh_handle->triangles_vbo = triangle_buffer;
#if TRACING_VERBOSE
    printf("finished uploading triangle indices\n");
#endif

    // wrap these up in a vertex array object.
    //--- is this encapsulation working for triangle indices?
#if TRACING_VERBOSE
    printf("preparing vertex attribute arrays\n");
#endif
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    // bind the attribute buffers to the vao
    for (int i = 0; i < NUM_ATTRIBUTE_TYPES; i++) {
        if (((mesh->vertex_format >> i) & 1) == 1) { // vertex format has attribute i set
            glBindBuffer(GL_ARRAY_BUFFER, mesh_handle->attribute_vbos[i]);
            glVertexAttribPointer(g_attribute_info[i].attribute_type, // location is currently set to the type index, so layout qualifiers need to match up the position in shaders.
                                  g_attribute_info[i].gl_size, // "gl_size" is the number of values per vertex, e.g. 3, 4.
                                  g_attribute_info[i].gl_type,
                                  GL_FALSE, // not normalized
                                  0,        // no stride (contiguous data in buffer)
                                  (void *) 0); // Buffer offset. Separate buffer objects for each attribute are being used.
            glEnableVertexAttribArray(g_attribute_info[i].attribute_type);
        }
    }
#if TRACING_VERBOSE
    printf("finished preparing vertex attribute arrays\n");
#endif
    // bind the triangle indices to the vao
    //--- is this in the right order?
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangle_buffer);
    // associate this vertex array object to the mesh handle.
    mesh_handle->vao = vao;
#if TRACING_VERBOSE
    printf("finished uploading mesh and associating it to mesh handle\n");
#endif
#undef TRACING
#undef TRACING_VERBOSE
}

void upload_and_free_mesh(MeshHandle *mesh_handle, Mesh *mesh)
{
    upload_mesh(mesh_handle, mesh);
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
        // fun fact: old C put declarations before statements, so we still can't follow a label with a declaration.
        switch (renderer->uniforms[i].type) {
            case UNIFORM_FLOAT:
                glUniform1f(renderer->uniforms[i].location,
                            renderer->uniforms[i].get_uniform_value().float_value);
            break;
            case UNIFORM_INT:
                glUniform1i(renderer->uniforms[i].location,
                            renderer->uniforms[i].get_uniform_value().int_value);
            break;
            case UNIFORM_MAT4X4:
                glUniformMatrix4fv(renderer->uniforms[i].location,
                                   1, // one matrix
                                   GL_FALSE, // no transpose ---may need to transpose. transposing this gets a freeze (???)
                                   renderer->uniforms[i].get_uniform_value().mat4x4_value); // pointer to matrix values
                //------ unsure, if buggy, check here
            break;
            default:
                fprintf(stderr, ERROR_ALERT "Renderer has a uniform which has an invalid uniform type, %d. (or this type is not accounted for by the uniform uploading function.)\n", renderer->uniforms[i].type);
                exit(EXIT_FAILURE);
        }
    }
}

void render_mesh(Renderer *renderer, MeshHandle *mesh_handle, GLenum primitive_mode)
{
    // vertex format compatibility: renderer must not require attribute types the
    // mesh does not have. However, if it does have extra, the renderer can just ignore them.
    if ((renderer->vertex_format & (~mesh_handle->vertex_format)) != 0) { // mesh handle's vertex format bitmask is not a superset of the renderer's
        fprintf(stderr, ERROR_ALERT "Attempted to render a mesh with renderer with incompatible vertex format.\n");
        fprintf(stderr, "\trenderer vertex format:\n");
        fprint_vertex_format(stderr, renderer->vertex_format);
        putc('\n', stderr);
        fprintf(stderr, "\tmesh handle vertex format:\n");
        fprint_vertex_format(stderr, mesh_handle->vertex_format);
        putc('\n', stderr);
        exit(EXIT_FAILURE);
    }
    bind_renderer(renderer);

    glBindVertexArray(mesh_handle->vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_handle->triangles_vbo);
    glDrawElements(primitive_mode,
                   3 * mesh_handle->num_triangles,
                   GL_UNSIGNED_INT,
                   (void *) 0);
}

void zero_init_renderer(Renderer *renderer)
{
    memset(renderer, 0, sizeof(Renderer));
    // if something is "zero initialized" to something other than 0, change this here.
}


Renderer *new_renderer_vertex_fragment(VertexFormat vertex_format, ShaderID vertex_shader_id, ShaderID fragment_shader_id)
{
    if (get_shader(vertex_shader_id) == NULL  ||  get_shader(fragment_shader_id) == NULL) {
        fprintf(stderr, ERROR_ALERT "Attempted to create a new renderer with an invalid shader id.\n");
        exit(EXIT_FAILURE);
    }
    RendererID id = create_renderer_id();
    g_renderer_table[id.table_index] = (Renderer *) calloc(1, sizeof(Renderer));
    mem_check(g_renderer_table[id.table_index]);
    Renderer *renderer = g_renderer_table[id.table_index];

    renderer->vertex_format = vertex_format;
    renderer->shaders[Vertex] = vertex_shader_id;
    renderer->shaders[Fragment] = fragment_shader_id;

    renderer->program_vram_id = glCreateProgram();
    renderer_compile_shaders(id);

    return renderer;
}

void renderer_compile_shaders(RendererID id)
{
    Renderer *renderer;
    if ((renderer = get_renderer(id)) == NULL) {
        fprintf(stderr, ERROR_ALERT "Attempted to compile shaders for invalid renderer ID.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < NUM_SHADER_TYPES; i++) {
        if (renderer->shaders[i] != NULL_SHADER_ID) {
            Shader *shader = get_shader(renderer->shaders[i]);
            if (shader == NULL) {
                fprintf(stderr, ERROR_ALERT "Attempted to compile shaders for renderer when it has an invalid shader ID.\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}


#define TRACING 0
void new_renderer_vertex_fragment(Renderer *renderer, VertexFormat vertex_format, char *vertex_shader_path, char *fragment_shader_path)
{


    /* Creates a new renderer only using vertex and fragment shaders. */
    zero_init_renderer(renderer);
#if TRACING
    printf("Creating new renderer with\n");
    printf("\tVertex shader path: %s\n", vertex_shader_path);
    printf("\tFragment shader path: %s\n", fragment_shader_path);
#endif
    renderer->vertex_format = vertex_format;
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
    //---- update this
    printf("Mesh handle:\n");
    printf("Vertex array object: %d\n", mesh_handle->vao);
    printf("Number of vertices: %d\n", mesh_handle->num_vertices);
    printf("Number of triangles: %d\n", mesh_handle->num_triangles);
    /* printf("Attached VBOs:\n"); */
    /* for (int i = 0; mesh_handle->vbos[i] != 0; i++) { */
    /*     printf("\t%d\n", mesh_handle->vbos[i]); */
    /* } */
}

void fprint_vertex_format(FILE *file, VertexFormat vertex_format)
{
    /* fprintf(file, "RAW: %d\n", vertex_format); */
    for (int i = 0; i < NUM_ATTRIBUTE_TYPES; i++) {
        fprintf(file, "%s: ", g_attribute_info[i].name);
        if (((vertex_format >> i) & 1) == 1) {
            putc('1', file);
        } else {
            putc('0', file);
        }
        putc('\n', file);
    }
}

void print_vertex_format(VertexFormat vertex_format)
{
    fprint_vertex_format(stdout, vertex_format);
}

void print_mesh(Mesh *mesh)
{
    printf("Mesh:\n");
    printf("vertex_format:\n");
    print_vertex_format(mesh->vertex_format);
    printf("num_vertices: %u\n", mesh->num_vertices);
    printf("num_triangles: %u\n", mesh->num_triangles);
}

//--- this shouldn't have to be done for every struct ...
void serialize_mesh_handle(FILE *file, MeshHandle *mesh_handle)
{
    //---- update this
    fprintf(file, "vao: %d\n", mesh_handle->vao);
    /* fprintf(file, "vbos: ["); //--- need standards for serialization */
    /* for (int i = 0; i < MAX_MESH_VBOS; i++) { */
    /*     if (mesh_handle->vbos[i] == 0) break; */
    /*     if (i != 0) fprintf(file, ", "); */
    /*     fprintf(file, "%d", mesh_handle->vbos[i]); */
    /* } */
    /* fprintf(file, "]\n"); */
    fprintf(file, "num_vertices: %u\n", mesh_handle->num_vertices);
    fprintf(file, "num_triangles: %u\n", mesh_handle->num_triangles);
}

Uniform *renderer_add_uniform(Renderer *renderer, char *name, UniformData (*get_uniform_value)(void), GLuint uniform_type)
{
    // uniform type: a GL_ type
    if (renderer->num_uniforms >= MAX_RENDERER_UNIFORMS) {
        fprintf(stderr, ERROR_ALERT "Too many uniforms added to renderer.\n");
        exit(EXIT_FAILURE);
    }
    Uniform *new_uniform = &renderer->uniforms[renderer->num_uniforms];
    
    if (strlen(name) > MAX_UNIFORM_NAME_LENGTH) {
        fprintf(stderr, ERROR_ALERT "Uniform name \"%s\" too long (MAX_UNIFORM_NAME_LENGTH: %d).\n", name, MAX_UNIFORM_NAME_LENGTH);
        exit(EXIT_FAILURE);
    }
    strncpy(new_uniform->name, name, MAX_UNIFORM_NAME_LENGTH);

    new_uniform->location = glGetUniformLocation(renderer->program, new_uniform->name);
    new_uniform->get_uniform_value = get_uniform_value;
    new_uniform->type = uniform_type;

    renderer->num_uniforms ++;
    return new_uniform;
}

//----remove this
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

void print_vertex_attribute_types(void)
{
    for (int i = 0; i < NUM_ATTRIBUTE_TYPES; i++) {
        // currently, i is the attribute type, but not using that for printing.
        printf("------------------------------------------------------------\n");
        printf("Vertex attribute type name: \"%s\"\n", g_attribute_info[i].name);
        printf("attribute_type: %d\n", g_attribute_info[i].attribute_type);
        printf("bitmask: ");
        for (int ii = 0; ii < 32; ii++) {
            if (ii == 32 - 1 - g_attribute_info[i].attribute_type) {
                putchar('1');
            } else {
                putchar('0');
            }
        }
        /* for (int ii = 0; ii < NUM_ATTRIBUTE_TYPES; ii++) { */
        /*     if (ii == NUM_ATTRIBUTE_TYPES - 1 - g_attribute_info[i].attribute_type) { */
        /*         putchar('1'); */
        /*     } else { */
        /*         putchar('0'); */
        /*     } */
        /* } */
        putchar('\n');
        printf("gl_type: %d\n", g_attribute_info[i].gl_type);
        printf("gl_size: %d\n", g_attribute_info[i].gl_size);
    }
}

//--------------------------------------------------------------------------------
// Loaders
//--------------------------------------------------------------------------------
void load_mesh_ply(Mesh *mesh, VertexFormat vertex_format, char *ply_filename, float size_factor)
{
    memset(mesh, 0, sizeof(Mesh));
    printf("Loading mesh from PLY file %s ...\n", ply_filename);

    mesh->vertex_format = vertex_format;

    PLY *ply = read_ply(ply_filename);

    if ((vertex_format & VERTEX_FORMAT_3) == 0) {
        fprintf(stderr, ERROR_ALERT "Attempted to load PLY mesh with invalid vertex format (no position attribute).\n");
        exit(EXIT_FAILURE);
    }
    
    char *pos_query = "[VERTEX|VERTICES|vertex|vertices|position|pos|positions|point|points]: \
float X|x|xpos|x_position|posx|position_x|x_coord|coord_x, \
float Y|y|ypos|y_position|posy|position_y|y_coord|coord_y, \
float Z|z|zpos|z_position|posz|position_z|z_coord|coord_z";
    int num_vertices;
    void *pos_data = ply_get(ply, pos_query, &num_vertices);
    mesh->attribute_data[ATTRIBUTE_TYPE_POSITION] = pos_data;
    mesh->num_vertices = num_vertices;

    if ((vertex_format & VERTEX_FORMAT_C) != 0) {
        char *color_query = "[VERTEX|VERTICES|vertex|vertices|position|pos|positions|point|points|COLOR|COLORS|COLOUR|COLOURS|color|colors|colour|colours]: \
float r|red|R|RED, \
float g|green|G|GREEN, \
float b|blue|B|BLUE";
        void *color_data = ply_get(ply, color_query, NULL);
        mesh->attribute_data[ATTRIBUTE_TYPE_COLOR] = color_data;
    }
    if ((vertex_format & ~VERTEX_FORMAT_3 & ~VERTEX_FORMAT_C) != 0) {
        fprintf(stderr, ERROR_ALERT "Attempted to extract mesh data with unsupported vertex format from PLY file.\n");
        exit(EXIT_FAILURE);
    }

#if 1
    printf("Loading face data.\n");
    
    // Triangles and face data. This is queried for, then it is made sure each face has 3 vertex indices,
    // then packs this data into the format used for meshes, with no counts (just ...|...|... etc.).
    //
    // This complication is because PLY really defines a more generic numerical list data format, while this only wants groups of threes for triangle indices.
    char *face_query = "[face|faces|triangle|triangles|tris|tri]: \
list int vertex_index|vertex_indices|indices|triangle_indices|tri_indices|index_list|indices_list";
    printf("Loaded face data.\n");
    int num_faces;
    void *face_data = ply_get(ply, face_query, &num_faces);
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
            /* printf("adding %u, ", ((uint32_t *) (face_data + face_data_offset))[0]); */
            memcpy(triangles + triangles_offset, face_data + face_data_offset, sizeof(uint32_t));
            face_data_offset += sizeof(uint32_t);
            triangles_offset = to_size;
        }
        /* printf("\n"); */
        /* getchar(); */
    }

    mesh->num_triangles = num_faces;
    mesh->triangles = (uint32_t *) triangles;
    free(face_data); // this is not stored in the mesh

#endif

    /* printf("================================================================================\n"); */
    /* printf("Finished loading PLY mesh.\n"); */
    /* printf("================================================================================\n"); */
    /* print_mesh(mesh); */
    /* for (int i = 0; i < mesh->num_vertices; i++) { */
    /*     printf("%f %f %f\n", ((float *) mesh->attribute_data[ATTRIBUTE_TYPE_POSITION])[3*i + 0], */
    /*                          ((float *) mesh->attribute_data[ATTRIBUTE_TYPE_POSITION])[3*i + 1], */
    /*                          ((float *) mesh->attribute_data[ATTRIBUTE_TYPE_POSITION])[3*i + 2]); */
    /* } */
    /* getchar(); */
    /* for (int i = 0; i < mesh->num_triangles; i++) { */
    /*     printf("%u %u %u\n", mesh->triangles[3*i + 0], */
    /*                          mesh->triangles[3*i + 1], */
    /*                          mesh->triangles[3*i + 2]); */
    /* } */

    //////////////////////////////////////////////////////////////////////////////////
    // Resizing here for now. Need mesh editing facilities! Or just scale transform!
    //////////////////////////////////////////////////////////////////////////////////
    for (int i = 0; i < mesh->num_vertices; i++) {
        ((float *) mesh->attribute_data[ATTRIBUTE_TYPE_POSITION])[3*i + 0] *= size_factor;
        ((float *) mesh->attribute_data[ATTRIBUTE_TYPE_POSITION])[3*i + 1] *= size_factor;
        ((float *) mesh->attribute_data[ATTRIBUTE_TYPE_POSITION])[3*i + 2] *= size_factor;
    }
}

