/*--------------------------------------------------------------------------------
   Rendering module.
Dependencies:
	resources:  Basic graphics objects are formed as resources, special kinds of shared objects tied to assets.
	dictionary: A basic key-value-pair file format used here for configuring graphics objects.
        images:     Images are used, so this is a basic dependency.
--------ply:        Really should not be a dependency (?).
--------------------------------------------------------------------------------*/
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
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
    /* printf("loading shader from path \"%s\" ...\n", path); */
    /* getchar(); */

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
    /* printf("Setting shader_id: %u\n", shader_id); */
    shader->shader_id = shader_id;
    return (void *) shader;
}
bool Shader_reload(ResourceHandle handle)
{
    //    Note on resource system:
    //      Reloads should probably be optional.
    // Is there a way to have a compilation on a separate ID then
    // swap over the compiled contents if it is a success? Could
    // have a dummy ID for each shader and compile twice so
    // ids don't go stale. Or just create a new ID to try each time,
    // and swap to it if successful. If things hold a resource handle
    // to a shader anyway, this is fine.
    //          --- trying this
    Shader *shader = resource_data(Shader, handle);
    char shader_path_buffer[1024];
    if (!resource_file_path(handle._path, "", shader_path_buffer, 1024)) return false;
    GLuint test_id = glCreateShader(gl_shader_type(shader->shader_type));
    if (!load_and_compile_shader(test_id, shader_path_buffer)) return false;
    // The compilation was successful on the test ID, so give this to the resource handle.
    shader->shader_id = test_id;
    return true;
}
/* void Shader_unload(void *resource) */
/* { */
/*     // Probably shouldn't do unloading yet, because I don't know how the resource */
/*     // system is going to trigger that anyway. */
/*     Shader *shader = (Shader *) resource; */
/*     glDeleteShader(shader->shader_id); */
/* } */
/* extern ResourceType Shader_RTID; */
/* typedef struct /1* Resource *1/ Shader_s { */
/*     ShaderType shader_type; */
/*     GraphicsID shader_id; */
/* } Shader; */

ResourceType GraphicsProgram_RTID;
void *GraphicsProgram_load(char *path)
{
#define load_error(STRING)\
    { fprintf(stderr, ( STRING )); fputc('\n', stderr); return NULL; }
    //----------------Fatal error messages are here for now for testing.
    // Path format:
    //      Drive/path/to/graphicsprogram
    // Associated build files:
    //      .../path/to/graphicsprogram.GraphicsProgram
    // This is a standard key-value pairs file with graphics program information.
    FILE *file = resource_file_open(path, ".GraphicsProgram", "r");
    if (file == NULL) load_error("Could not open graphics program file.");
    Dictionary *dict = dictionary_read(file);
    if (dict == NULL) load_error("Could not read dictionary for graphics program.");
    
    // Read in entries from the dictionary.
    const int buf_size = 512;
    char buf[buf_size];

    VertexFormat vertex_format;
    char vertex_shader_path[buf_size];
    char fragment_shader_path[buf_size];
    #define dict_try(STRING)\
        { if (!dict_get(dict, ( STRING ), buf, buf_size)) {\
                fprintf(stderr, "Could not find " #STRING ".\n");\
                return NULL;\
        } }
    dict_try("vertex_format");
    vertex_format = string_to_VertexFormat(buf);
    if (vertex_format == VERTEX_FORMAT_NONE) load_error("Invalid vertex format");
    dict_try("vertex_shader");
    strncpy(vertex_shader_path, buf, buf_size);
    dict_try("fragment_shader");
    strncpy(fragment_shader_path, buf, buf_size);
#if 0
    dict_try("type");
    GraphicsProgramType graphics_program_type;
    if (strcmp(buf, "StandardView") == 0) {
        graphics_program_type = GRAPHICS_PROGRAM_STANDARD_VIEW;
    } else if (strcmp(buf, "Standard2D") == 0) {
        graphics_program_type = GRAPHICS_PROGRAM_STANDARD_2D;
    } else {
        load_error("Invalid graphics program type.");
    }
#endif

#if 0
    char uniform_string[buf_size];
    if (!dict_get(dict, "uniforms", buf, buf_size)) {
        fprintf(stderr, "COULD NOT FIND uniforms\n");
        return NULL;
    }
    strncpy(uniform_string, buf, buf_size);
#endif

    GraphicsProgram *program = (GraphicsProgram *) calloc(1, sizeof(GraphicsProgram));
    mem_check(program);
    program->program_id = glCreateProgram();
    if (program->program_id == 0) load_error("Could not create program ID.");
    program->program_type = GRAPHICS_PROGRAM_VF; // Only using vertex and fragment shaders.
    program->vertex_format = vertex_format;
    /* program->program_type = graphics_program_type; //reused program_type... was for which shader stages are a part of it. */

    //////////////////////////////////////////////////////////////////////////////////
    // If create new resource handles !!! REMEMBER TO FREE if resource load fails.
    program->shaders[Vertex] = new_resource_handle(Shader, vertex_shader_path);
    program->shaders[Fragment] = new_resource_handle(Shader, fragment_shader_path);

    // Resources for shaders have been associated. Now, attach them and link them to this GraphicsProgram object.
    // The shader resource dereferences will trigger shader loads and compilation.
    glAttachShader(program->program_id, resource_data(Shader, program->shaders[Vertex])->shader_id);
    glAttachShader(program->program_id, resource_data(Shader, program->shaders[Fragment])->shader_id);
    // link_shader_program links and does link-error checking.
    link_shader_program(program->program_id);
    // Detach the shaders so they can be deleted if needed (but for now, as resources, they are still loaded resources available
    // for linking into other graphics programs).
    glDetachShader(program->program_id, resource_data(Shader, program->shaders[Vertex])->shader_id);
    glDetachShader(program->program_id, resource_data(Shader, program->shaders[Fragment])->shader_id);

#if 0
    GLuint uniform_block_index;
    if (program->program_type == GRAPHICS_PROGRAM_STANDARD_VIEW) {
        uniform_block_index = glGetUniformBlockIndex(program->program_id, "StandardView");
        if (uniform_block_index == GL_INVALID_INDEX) load_error("Could not find standard uniform block \"StandardView\".");
        // The standard uniform block (time, mvp matrix, aspect ratio, dimensions, stuff like that) is now bound to this program.
        glUniformBlockBinding(program->program_id, uniform_block_index, UNIFORM_BLOCK_STANDARD);
    } else if (program->program_type == GRAPHICS_PROGRAM_STANDARD_2D) {
        GLuint uniform_block_index = glGetUniformBlockIndex(program->program_id, "Standard2D");
        if (uniform_block_index == GL_INVALID_INDEX) load_error("Could not find standard uniform block \"Standard2D\".");
        // The standard uniform block (time, mvp matrix, aspect ratio, dimensions, stuff like that) is now bound to this program.
    } else {
        load_error("Invalid graphics program type got through.");
    }
    glUniformBlockBinding(program->program_id, uniform_block_index, UNIFORM_BLOCK_STANDARD);
#endif
    // Now that the program has been compiled, gather the uniform locations.
    // Example in .GraphicsProgram file:
    // uniforms: mat4x4 mvp_matrix, float aspect_ratio, float frand, vec3 model_pos
#if 0
    /* A way to query the standard dictionary file for information, store
     * as certain types, give error/malformation information, and facilities
     * for storing things further than key-value-pairs, or list-type values,
     * tabbed lists, ... , is really needed instead of hardcoding string processing
     * and retrieval from the dictionary in each thing that uses one.
     */
    char *entry = uniform_string;
    while (*entry != '\0') {
        char *end = strchr(entry, ',');
        if (end == NULL) end = strchr(entry, '\0');
        while (isspace(*entry)) entry++;
        
        char *sep = strchr(entry, ' ');
        if (sep == NULL || sep >= end) load_error("Bad list of uniforms given.");
        char buf[512];
        strncpy(buf, entry, sep - entry);
        buf[sep - entry] = '\0';
            /* printf("Got type \"%s\"\n", buf); */
            /* getchar(); */
        UniformType uniform_type = string_to_UniformType(buf);
        if (uniform_type == UNIFORM_NONE) load_error("Invalid uniform type given.");
        strncpy(buf, sep + 1, end - (sep + 1));
        buf[end - (sep + 1)] = '\0';
            /* printf("Got name \"%s\"\n", buf); */
            /* getchar(); */

        // Add this uniform to the graphics program.
        GraphicsProgram_add_uniform(program, uniform_type, buf);

        if (*end == '\0') break;
        entry = end + 1;
    }
#endif

    destroy_dictionary(dict);
    /* printf("!!! program type: %u\n", program->program_type); */
    /* getchar(); */
    return (void *) program;
#undef load_error
#undef dict_try
}
bool GraphicsProgram_reload(ResourceHandle handle)
{
    //---not rereading the .GraphicsProgram file.
    GraphicsProgram *program = resource_data(GraphicsProgram, handle);
    GLuint test_id = glCreateProgram();

    printf("program type: %u\n", program->program_type);
    for (int i = 0; i < NUM_SHADER_TYPES; i++) {
        /* printf("%u", (program->program_type & (1 << i)) != 0 ? 1 : 0); */
        printf("%u", (program->program_type >> i) & 0x1);
    }
    printf("\n");
    
#define IS_ACTIVE(SHADER_TYPE_INDEX) ( (program->program_type & (1 << i)) != 0 )
    for (int i = 0; i < NUM_SHADER_TYPES; i++) {
        if (IS_ACTIVE(i)) { // program type bitmask contains i'th shader type, and the resource handle is non-null.
            if (!Shader_reload(program->shaders[i])) return false;
            glAttachShader(test_id, resource_data(Shader, program->shaders[i])->shader_id);
        }
    }
    if (link_shader_program(test_id)) {
        // The compilation on the test id worked. Make the graphicsprogram resource handle hold the new ID.
        program->program_id = test_id;
        for (int i = 0; i < NUM_SHADER_TYPES; i++) {
            if (IS_ACTIVE(i)) {
                glDetachShader(test_id, resource_data(Shader, program->shaders[i])->shader_id);
            }
        }
        return true;
    }
    return false;
#undef IS_ACTIVE
}


ResourceType Mesh_RTID;
#define load_error(STRING)\
{\
    fprintf(stderr, "ERROR LOADING MESH: %s\n", ( STRING ));\
    exit(EXIT_FAILURE);\
}
void *Mesh_load(char *path)
{
#if 0
    FILE *image_file = resource_file_open(path, ".Mesh.image", "rb");
    if (image_file != NULL) {
        // ...
    }
#endif
    FILE *file = resource_file_open(path, ".Mesh", "r");
    if (file == NULL) load_error("file can't open");
    Dictionary *dict = dictionary_read(file);
    if (dict == NULL) load_error("dictionary can't be made");
    const int buf_size = 512;
    char buf[buf_size];
    if (!dict_get(dict, "vertex_format", buf, buf_size)) load_error("vertex_format cannot be found");
    VertexFormat vertex_format = string_to_VertexFormat(buf);
    if (vertex_format == VERTEX_FORMAT_NONE) load_error("vertex_format is invalid");
    if (!dict_get(dict, "filetype", buf, buf_size)) load_error("filetype cannot be found");
    if (strncmp(buf, "ply", buf_size) == 0) {
        // Loading the mesh from a PLY file.
        FILE *ply_file = resource_file_open(path, ".ply", "r");
        if (ply_file == NULL) load_error("cannot open resource PLY file");
        MeshData mesh_data;
        load_mesh_ply(&mesh_data, vertex_format, ply_file);
        Mesh *mesh = (Mesh *) calloc(1, sizeof(Mesh));
        mem_check(mesh);
        upload_mesh(mesh, &mesh_data);
        //////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////
        // DESTROY THE MESH DATA !!!!
        //////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////
        printf("GOT A MESH!!!\n");
        return mesh;
    } else {
        // Invalid mesh filetype or it is not supported.
        load_error("invalid mesh filetype");
    }
}
#undef load_error

ResourceType Artist_RTID;
void *Artist_load(char *path)
{
    // An Artist is a "virtual resource".
    Artist *artist = (Artist *) calloc(1, sizeof(Artist));
    mem_check(artist);
    return artist;
}

void init_resources_rendering(void)
{
    add_resource_type(Shader);
    add_resource_type(GraphicsProgram);
    add_resource_type(Mesh);
    add_resource_type(Artist); // "Virtual" resource
    add_resource_type(Texture);
}

//================================================================================
//   Artist
//================================================================================
void GraphicsProgram_add_uniform(GraphicsProgram *program, UniformType uniform_type, char *name)
{
    if (strlen(name) > MAX_UNIFORM_NAME_LENGTH) {
        fprintf(stderr, ERROR_ALERT "Uniform name \"%s\" too long (MAX_UNIFORM_NAME_LENGTH: %d).\n", name, MAX_UNIFORM_NAME_LENGTH);
        exit(EXIT_FAILURE);
    }
    if (program->uniform_array == NULL) {
        program->num_uniforms = 1;
        program->uniform_array = (Uniform *) calloc(program->num_uniforms, sizeof(Uniform));
        mem_check(program->uniform_array);
    } else {
        program->num_uniforms ++;
        program->uniform_array = (Uniform *) realloc(program->uniform_array, program->num_uniforms * sizeof(Uniform));
        mem_check(program->uniform_array);
    }
    Uniform *new_uniform = &program->uniform_array[program->num_uniforms - 1];
    strncpy(new_uniform->name, name, MAX_UNIFORM_NAME_LENGTH);

    /* printf("Program ID: %d, name: %s\n", (int) program->program_id, name); */
    /* getchar(); */
    new_uniform->location = glGetUniformLocation(program->program_id, name);
    if (new_uniform->location < 0) {
        fprintf(stderr, ERROR_ALERT "Failed to find location for uniform \"%s\".\n", new_uniform->name);
        exit(EXIT_FAILURE);
    }
    new_uniform->type = uniform_type;
    /* printf("Added new uniform: %s, %d\n", new_uniform->name, new_uniform->type); */
    /* getchar(); */
}

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

    new_uniform->location = glGetUniformLocation(resource_data(GraphicsProgram, artist->graphics_program)->program_id, new_uniform->name);
    if (new_uniform->location < 0) {
        fprintf(stderr, ERROR_ALERT "Failed to find location for uniform \"%s\".\n", new_uniform->name);
        getchar();
    }
    /* printf("%s location: %d\n", new_uniform->name, new_uniform->location); */
    /* getchar(); */
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


/*
Usage of an artist for drawing.

So far, an "artist" is a thin wrapper around a graphics program. Why should this be used?
One reason is the complication of having to set up a graphics program with information
about the application using it, callback functions etc. But it is a shared resource, a compiled
~object file~. So, rather let the graphics program resource expose its uniform-interface.
GraphicsProgram:
    Uniform interface (required/non-required)
    Minimal vertex format
    ---and stuff for textures, buffers, whatever may be needed in this encapsulation later.


"Standard" uniforms. A uniform variable is a general thing, and will have many uses,
but there is still a standard core of stuff that needs to be uploaded on a draw
call for a 3D mesh.

What about having an artist hold an optional "prepare" function. This will be
called when the artist is bound, and use the same functions for uniform uploading
("preparing") as would be used after binding for standard uniform uploads. This means
that non-standard uniforms used in its graphics program may be prepared
(or ignored, if there is an initialized value in the shaders/if there is a signifier in the .GraphicsProgram file).

Then, there could even be error messages for program misuse, and I think this would be very
important for practical use of this higher-level graphics-object stuff. In the .GraphicsProgram file,
the list of uniforms may be
    uniforms: float! aspect_ratio, mat4x4! mvp_matrix, float! multiplier, bool grayscale
Here, aspect_ratio and mvp_matrix are required standard uniforms. multiplier is a required non-standard uniform,
and grayscale is an optional non-standard uniform, possibly initializing to false in the shader.

Now when an artist is bound, it stores flags for each of its graphics program's uniforms (at the start, so these
are altered by the call to the artist's .prepare function).
The usage of a draw call is:
Bind an artist, prepare standard uniforms, draw call.
At a draw call, the flags are checked. If there is any required uniform with an inactive flag, an error is raised.
This says what graphics program, what uniform, its type/name, etc., so this can quickly be looked at.

This will lead you to either change the standard uploads (their names, add more), or
go look at the way you initialized the artist, and hook up the required uniforms.

What about superfluous uniforms? Uniform aliases? Common names for the mvp matrix, etc.?
If a graphics program is thought of to encapsulate the minimal requirements for preparing a draw call under this program,
then
    Does the mesh have a compatible vertex format? (allowing superfluous vertex attributes)
    Does the artists+usage-of-the-draw-call provide the neccessary uniforms? (allowing superfluous uniforms)
    ...
    etc., Does the material/... have required texture information? Or whatever else.

Also, since the .prepare function is called first on binding the artist, you will not need to worry about
the artist preparing its own standard uniforms, as they will be overwritten.

-
some outline (not how it should work)

for_aspect(Camera, camera)
    Matrix4x4f vp_matrix = camera->projection_matrix; // a "camera" as an entity w/ aspect will have a lens and a clipping rectangle. Not taking this fully into account yet.
    right_multiply_matrix4x4f(vp_matrix, Transform_matrix(get_sibling_aspect(camera, Transform)));

    for_aspect(Body, body)
        Transform *transform = get_sibling_aspect(body, Transform);

        Matrix4x4f mvp_matrix = vp_matrix;
        right_multiply_matrix4x4f(mvp_matrix, Transform_matrix(transform));

        Artist *artist = resource_data(Artist, body->artist);

        Artist_bind(artist);
        Artist_prepare_float(artist, "aspect_ratio", g_aspect_ratio);
        Artist_prepare_mat4x4(artist, "mvp_matrix", mvp_matrix.vals);
        Artist_prepare_vec3(artist, "model_position", vec3(transform->x, transform->y, transform->z));
        Artist_draw_mesh(artist, resource_data(Mesh, body->mesh));
    end_for_aspect()
end_for_aspect()
*/

#if 0
void Artist_prepare_float(Artist *artist, char *name, float val)
{
    GraphicsProgram *program = resource_data(GraphicsProgram, artist->graphics_program);

    glUniform1f(artist->uniform_array[i].location,
                artist->uniform_array[i].getter.float_getter());
}


void *Artist_bind(Artist *artist)
{
    /* Set the graphics context up with what is needed to draw with this artist.
     */
    GraphicsProgram *program = resource_data(GraphicsProgram, artist->graphics_program);
    glUseProgram(program->program_id);
    if (artist->prepare != NULL) artist->prepare(artist);
}
#endif

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
        fprintf(stderr, "%u\n", mesh->vertex_format);
        exit(EXIT_FAILURE);
    }
    Artist_bind(artist);
    glBindVertexArray(mesh->vertex_array_id);
    //---Note, reordered this because I think I was not properly associating the triangle index buffer to the vao.
    // If there is a problem, maybe rethink that, but I don't think the triangle list needs to be bound each time here,
    // rather just when filling the vao with state.
    /* glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->triangles_id); */
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
    VertexFormat vertex_format = 0;
#define casemap(CHAR,VF) case ( CHAR ):\
        vertex_format |= ( VF ); break;
    for (int i = 0; string[i] != '\0'; i++) {
        switch (string[i]) {
            casemap('3', VERTEX_FORMAT_3);
            casemap('C', VERTEX_FORMAT_C);
            casemap('N', VERTEX_FORMAT_N);
            casemap('U', VERTEX_FORMAT_U);
            default:
                return VERTEX_FORMAT_NONE;
        }
    }
    return vertex_format;
#undef casemap
}
UniformType string_to_UniformType(char *string)
{
    if (strcmp(string, "float") == 0) return UNIFORM_FLOAT;
    if (strcmp(string, "int") == 0) return UNIFORM_INT;
    if (strcmp(string, "mat4x4") == 0) return UNIFORM_MAT4X4F;
    return UNIFORM_NONE;
}

//--------------------------------------------------------------------------------
// Mesh stuff
//--------------------------------------------------------------------------------
const AttributeInfo g_attribute_info[NUM_ATTRIBUTE_TYPES] = {
    { ATTRIBUTE_TYPE_POSITION, "vPosition", GL_FLOAT, 3 },
    { ATTRIBUTE_TYPE_COLOR, "vColor", GL_FLOAT, 3 },
    { ATTRIBUTE_TYPE_NORMAL, "vNormal", GL_FLOAT, 3},
}; //----add texture coordinates.

void upload_mesh(Mesh *mesh, MeshData *mesh_data)
{
    /* Given a a Mesh to be initialized, and a MeshData consisting of vertex attribute data and other mesh information
     * in application memory, upload this data to video memory, and fill out the mesh structure (the handle) with IDs and
     * information enough to own these graphics objects.
     */
    // Make sure the mesh handle is zero initialized before filling.
    memset(mesh, 0, sizeof(Mesh));
    // Copy over basic properties.
    mesh->num_triangles = mesh_data->num_triangles;
    mesh->num_vertices = mesh_data->num_vertices;
    mesh->vertex_format = mesh_data->vertex_format;

    // Upload vertex attribute data and associate to the mesh handle.
    for (int i = 0; i < NUM_ATTRIBUTE_TYPES; i++) {
        if (((mesh_data->vertex_format >> i) & 1) == 1) { // vertex format has attribute i set
	    // Upload this vertex attribute data to VRAM and give the ID to the mesh.	
            if (mesh_data->attribute_data[i] == NULL) {
                fprintf(stderr, ERROR_ALERT "Attempted to upload mesh which does not have data for one of its attributes.\n");
                exit(EXIT_FAILURE);
            }
            GLuint attribute_buffer;
            glGenBuffers(1, &attribute_buffer);
            glBindBuffer(GL_ARRAY_BUFFER, attribute_buffer);
            glBufferData(GL_ARRAY_BUFFER,
                         g_attribute_info[i].gl_size * gl_type_size(g_attribute_info[i].gl_type) * mesh_data->num_vertices,
                         mesh_data->attribute_data[i],
                         GL_STATIC_DRAW);
            // Give this buffer ID to the mesh.
            mesh->attribute_buffer_ids[i] = attribute_buffer;
        }
    }
    // Upload triangle-index/triangle-list data and give it to the mesh.
    GLuint triangle_buffer;
    glGenBuffers(1, &triangle_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangle_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3*sizeof(uint32_t) * mesh_data->num_triangles, mesh_data->triangles, GL_STATIC_DRAW);
    mesh->triangles_id = triangle_buffer;
    // Finished triangle indices.

    // Create a vertex array object (VAO) and associate the vertex attribute arrays to it, then give it to the mesh:
    // , creating and binding a new VAO ID
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    // , binding the attribute buffers and triangle index buffer to the vertex array object
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangle_buffer);
    for (int i = 0; i < NUM_ATTRIBUTE_TYPES; i++) {
        if (((mesh_data->vertex_format >> i) & 1) == 1) { // vertex format has attribute i set
            glBindBuffer(GL_ARRAY_BUFFER, mesh->attribute_buffer_ids[i]);
            glVertexAttribPointer(g_attribute_info[i].attribute_type, // location is currently set to the type index, so layout qualifiers need to match up the position in shaders.
                                  g_attribute_info[i].gl_size, // "gl_size" is the number of values per vertex, e.g. 3, 4.
                                  g_attribute_info[i].gl_type,
                                  GL_FALSE, // not normalized
                                  0,        // no stride (contiguous data in buffer)
                                  (void *) 0); // Buffer offset. Separate buffer objects for each attribute are being used.
            glEnableVertexAttribArray(g_attribute_info[i].attribute_type);
        }
    }
    // , and finally, give this VAO ID to the mesh.
    mesh->vertex_array_id = vao;
}
