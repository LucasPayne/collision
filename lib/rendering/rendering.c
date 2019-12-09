/*--------------------------------------------------------------------------------
   Rendering module.
Dependencies:
	resources:  Basic graphics objects are formed as resources, special kinds of shared objects tied to assets.
	dictionary: A basic key-value-pair file format used here for configuring graphics objects.
--------ply:        Really should not be a dependency.
--------------------------------------------------------------------------------*/
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "helper_definitions.h"
#include "resources.h"
#include "ply.h" //----------Remove this dependency.
#include "rendering.h"
#include "dictionary.h"


// Rendering resources
//================================================================================
ResourceType Shader_RTID;
void *Shader_load(char *path)
{
    // Path format:
    //      Drive/path/to/shader.{vert,geom,frag}
    // Corresponds directly to a GLSL shader source file.
    ShaderType shader_type;
    char *suffix;
    // Want to keep the shader asset as a simple file, so infer the shader type from the path. 
    if ((suffix = strrchr(path, '.')) == NULL) return NULL;
    if (strcmp(suffix + 1, "vert") == 0) {
        shader_type = Vertex;
    } else if (strcmp(suffix + 1, "frag") == 0) {
        shader_type = Fragment;
    } else if (strcmp(suffix + 1, "geom") == 0) {
        shader_type = Geometry;
    } else {
        return NULL;
    }
    GLenum gl_shader_type = gl_shader_type(shader_type);
    if (gl_shader_type == 0) return NULL;
    GraphicsID shader_id = glCreateShader(gl_shader_type);
    printf("created id: %d\n", shader_id);
    // Load the shader source from the physical path, and attempt to compile it.
    char shader_path_buffer[1024];
    if (!resource_file_path(path, "", shader_path_buffer, 1024)) {
        fprintf(stderr, "FAILED TO GET SHADER FILE PATH\n");
        exit(EXIT_FAILURE);
        return NULL;
    }
    printf("Shader path: \"%s\"\n", shader_path_buffer);
    if (!load_and_compile_shader(shader_id, shader_path_buffer)) {
        fprintf(stderr, "FAILED TO COMPILE SHADER\n");
        exit(EXIT_FAILURE);
        return NULL;
    }
    // The shader has been compiled and associated to the created shader ID.
    // Create the Shader resource and return it.
    Shader *shader = (Shader *) calloc(1, sizeof(Shader));
    mem_check(shader);
    shader->shader_type = shader_type;
    printf("Setting shader_id: %u\n", shader_id);
    shader->shader_id = shader_id;
    return (void *) shader;
}
ResourceType GraphicsProgram_RTID;
void *GraphicsProgram_load(char *path)
{
    //----------------Fatal error messages are here for now for testing.
    // Path format:
    //      Drive/path/to/graphicsprogram
    // Associated build files:
    //      .../path/to/graphicsprogram.GraphicsProgram
    // This is a standard key-value pairs file with graphics program information.
    FILE *file = resource_file_open(path, ".GraphicsProgram", "r");
    if (file == NULL) {
        fprintf(stderr, "COULD NOT OPEN GRAPHICS PROGRAM FILE\n");
        return NULL;
    }
    Dictionary *dict = dictionary_read(file);
    if (dict == NULL) {
        fprintf(stderr, "COULD NOT READ DICTIONARY FOR GRAPHICS PROGRAM\n");
        return NULL;
    }
    
    // Read in entries from the dictionary.
    const int buf_size = 512;
    char buf[buf_size];

    VertexFormat vertex_format;
    char vertex_shader_path[buf_size];
    char fragment_shader_path[buf_size];

    if (!dict_get(dict, "vertex_format", buf, buf_size)) {
        fprintf(stderr, "COULD NOT FIND vertex_format\n");
        return NULL;
    }
    vertex_format = string_to_VertexFormat(buf);
    if (vertex_format == VERTEX_FORMAT_NONE) {
        fprintf(stderr, "INVALID VERTEX FORMAT \"%s\"\n", buf);
        return NULL;
    }
    if (!dict_get(dict, "vertex_shader", buf, buf_size)) {
        fprintf(stderr, "COULD NOT FIND vertex_shader\n");
        return NULL;
    }
    strncpy(vertex_shader_path, buf, buf_size);
    if (!dict_get(dict, "fragment_shader", buf, buf_size)) {
        fprintf(stderr, "COULD NOT FIND fragment_shader\n");
        return NULL;
    }
    strncpy(fragment_shader_path, buf, buf_size);

    GraphicsProgram *program = (GraphicsProgram *) calloc(1, sizeof(GraphicsProgram));
    mem_check(program);
    program->program_id = glCreateProgram();
    if (program->program_id == 0) return NULL;
    program->program_type = GRAPHICS_PROGRAM_VF; // Only using vertex and fragment shaders.
    program->vertex_format = vertex_format;

    // If create new resource handles !!! REMEMBER TO FREE if resource load fails. So, adding them last.
    program->shaders[Vertex] = new_resource_handle(Shader, vertex_shader_path);
    program->shaders[Fragment] = new_resource_handle(Shader, fragment_shader_path);

    // Resources for shaders have been associated. Now, attach them and link them to this GraphicsProgram object.
    glAttachShader(program->program_id, resource_data(Shader, program->shaders[Vertex])->shader_id);
    glAttachShader(program->program_id, resource_data(Shader, program->shaders[Fragment])->shader_id);
    link_shader_program(program->program_id);

    destroy_dictionary(dict);
    return (void *) program;
}
ResourceType Mesh_RTID;
void *Mesh_load(char *path)
{
#if 0
    FILE *image_file = resource_file_open(path, ".Mesh.image", "rb");
    if (image_file != NULL) {
        // ...
    }
#endif
    FILE *file = resource_file_open(path, ".Mesh", "r");
    if (file == NULL) goto load_error;
    Dictionary *dict = dictionary_read(file);
    if (dict == NULL) goto load_error;
    const int buf_size = 512;
    char buf[buf_size];
    if (!dict_get(dict, "vertex_format", buf, buf_size)) goto load_error;
    VertexFormat vertex_format = string_to_VertexFormat(buf);
    if (vertex_format == VERTEX_FORMAT_NONE) goto load_error;
    if (!dict_get(dict, "filetype", buf, buf_size)) goto load_error;
    if (strncmp(buf, "ply", buf_size)) {
        // Loading the mesh from a PLY file.
        MeshData mesh_data;
        FILE *ply_file = resource_file_open(path, ".ply", "r");
        if (ply_file == NULL) goto load_error;
        load_mesh_ply(&mesh_data, vertex_format, ply_file);
        
    } else {
        // Invalid mesh filetype or it is not supported.
        goto load_error;
    }
load_error:
    fprintf("ERROR LOADING MESH\n");
    exit(EXIT_FAILURE);
}

void init_resources_rendering(void)
{
    add_resource_type(Shader);
    add_resource_type(GraphicsProgram);
    add_resource_type(Mesh);
}

//================================================================================
//   Artist
//================================================================================
void Artist_add_uniform(Artist *artist, char *name, UniformGetter getter, UniformType uniform_type)
{
    if (strlen(name) > MAX_UNIFORM_NAME_LENGTH) {
        fprintf(stderr, ERROR_ALERT "Uniform name \"%s\" too long (MAX_UNIFORM_NAME_LENGTH: %d).\n", name, MAX_UNIFORM_NAME_LENGTH);
        exit(EXIT_FAILURE);
    }

    if (artist->uniform_array == NULL) {
        artist->num_uniforms = 1;
        artist->uniform_array = (Uniform *) calloc(artist->num_uniforms, sizeof(Uniform));
        mem_check(artist->uniform_array);
    } else {
        artist->num_uniforms ++;
        artist->uniform_array = (Uniform *) realloc(artist->uniform_array, artist->num_uniforms * sizeof(Uniform));
        mem_check(artist->uniform_array);
    }
    Uniform *new_uniform = &artist->uniform_array[artist->num_uniforms - 1];
    strncpy(new_uniform->name, name, MAX_UNIFORM_NAME_LENGTH);

    new_uniform->location = glGetUniformLocation(resource_data(GraphicsProgram, artist->graphics_program), new_uniform->name);
    new_uniform->getter = getter;
    new_uniform->type = uniform_type;
}

#define null_check(UNION_ITEM)\
{\
    if (( UNION_ITEM ) == NULL) {\
        fprintf(stderr, ERROR_ALERT "Haven't initialized a uniform value-getting function for artist.\n");\
        exit(EXIT_FAILURE);\
    }\
}
void Artist_bind(Artist *artist)
{
    /* Set the graphics context up with what is needed to draw with this artist.
     * This gets and uploads the attached uniforms and binds the associated graphics
     * program to the context. */
    glUseProgram(resource_data(GraphicsProgram, artist->graphics_program)->program_id);
    for (int i = 0; i < artist->num_uniforms; i++) {
        switch (artist->uniform_array[i].type) {
            case UNIFORM_FLOAT:
                null_check(artist->uniform_array[i].getter.float_getter);
                glUniform1f(artist->uniform_array[i].location,
                            artist->uniform_array[i].getter.float_getter());
            break;
            case UNIFORM_INT:
                null_check(artist->uniform_array[i].getter.int_getter);
                glUniform1i(artist->uniform_array[i].location,
                            artist->uniform_array[i].getter.int_getter());
            break;
            case UNIFORM_MAT4X4F:
                null_check(artist->uniform_array[i].getter.mat4x4f_getter);
                glUniformMatrix4fv(artist->uniform_array[i].location,
                                   1,        // Number of matrices
                                   GL_FALSE, // Transpose/no-transpose
                                   artist->uniform_array[i].getter.mat4x4f_getter().vals);
            break;
            default:
                fprintf(stderr, ERROR_ALERT "Artist has a uniform which has an invalid uniform type, %d. (or this type is not accounted for yet.)\n", artist->uniform_array[i].type);
                exit(EXIT_FAILURE);
        }
    }
}
#undef null_check

void Artist_draw_mesh(Artist *artist, Mesh *mesh)
{
    if ((resource_data(GraphicsProgram, artist->graphics_program)->vertex_format & (~mesh->vertex_format)) != 0) { // mesh's vertex format bitmask is not a superset of the graphics program's
        fprintf(stderr, ERROR_ALERT "Attempted to render a mesh with artist with incompatible vertex format.\n");
        exit(EXIT_FAILURE);
    }
    Artist_bind(artist);
    glBindVertexArray(mesh->vertex_array_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->triangles_id);
    glDrawElements(GL_TRIANGLES,
                   3 * mesh->num_triangles,
                   GL_UNSIGNED_INT,
                   (void *) 0);
}

//--------------------------------------------------------------------------------
// Loaders
//--------------------------------------------------------------------------------
void load_mesh_ply(MeshData *mesh, VertexFormat vertex_format, FILE *file)
{
    memset(mesh, 0, sizeof(Mesh));
    mesh->vertex_format = vertex_format;

    PLY *ply = read_ply(file);

    if ((vertex_format & VERTEX_FORMAT_3) == 0) {
        fprintf(stderr, ERROR_ALERT "Attempted to load PLY mesh with invalid vertex format (no position attribute).\n");
        exit(EXIT_FAILURE);
    }
    
    char *pos_query = "[VERTEX|VERTICES|vertex|vertices|position|pos|positions|point|points]: \
float X|x|xpos|x_position|posx|position_x|x_coord|coord_x, \
float Y|y|ypos|y_position|posy|position_y|y_coord|coord_y, \
float Z|z|zpos|z_position|posz|position_z|z_coord|coord_z";
    int num_vertices;
    void *pos_data = ply_get(file, ply, pos_query, &num_vertices);
    mesh->attribute_data[ATTRIBUTE_TYPE_POSITION] = pos_data;
    mesh->num_vertices = num_vertices;

    if ((vertex_format & VERTEX_FORMAT_C) != 0) {
        char *color_query = "[VERTEX|VERTICES|vertex|vertices|position|pos|positions|point|points|COLOR|COLORS|COLOUR|COLOURS|color|colors|colour|colours]: \
float r|red|R|RED, \
float g|green|G|GREEN, \
float b|blue|B|BLUE";
        void *color_data = ply_get(file, ply, color_query, NULL);
        mesh->attribute_data[ATTRIBUTE_TYPE_COLOR] = color_data;
    }
    if ((vertex_format & ~VERTEX_FORMAT_3 & ~VERTEX_FORMAT_C) != 0) {
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
}

//--------------------------------------------------------------------------------
// Helper, strings, serialization, ...
//--------------------------------------------------------------------------------
VertexFormat string_to_VertexFormat(char *string)
{
    if (strcmp(string, "3") == 0) return VERTEX_FORMAT_3;
    if (strcmp(string, "3C") == 0) return VERTEX_FORMAT_3C;
    if (strcmp(string, "3U") == 0) return VERTEX_FORMAT_3U;
    if (strcmp(string, "3CU") == 0) return VERTEX_FORMAT_3CU;
    if (strcmp(string, "3UC") == 0) return VERTEX_FORMAT_3CU;
    return VERTEX_FORMAT_NONE;
}

