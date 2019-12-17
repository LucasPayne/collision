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
#include "helper_definitions.h"
#include "resources.h"
#include "ply.h" //----------Remove this dependency.
#include "rendering.h"
#include "dictionary.h"

static int g_num_shader_blocks = 0;
ShaderBlockInfo g_shader_blocks[MAX_NUM_SHADER_BLOCKS];

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
            default:
                return VERTEX_FORMAT_NONE;
        }
    }
    return vertex_format;
#undef casemap
}

static bool query_val_unsigned_int(char *string, void *data)
{
    // move to dictionary's basic types (built in to querier)
    unsigned int ui;
    if (sscanf(string, "%u", &ui) == EOF) return false;
    memcpy(data, &ui, sizeof(unsigned int));
    return true;
}
static bool query_val_VertexFormat(char *string, void *data)
{
    VertexFormat vf = string_to_VertexFormat(string);
    if (vf == VERTEX_FORMAT_NONE) return false;
    memcpy(data, &vf, sizeof(vf));
    return true;
}
void dict_query_rules_rendering(DictQuerier *q)
{
    dict_query_rule_add(q, "VertexFormat", query_val_VertexFormat);
    
    // move to dictionary's basic types 
    dict_query_rule_add(q, "unsigned int", query_val_unsigned_int);
}

/*================================================================================
  Shader resource
================================================================================*/
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
        /* print_vertex_format(mesh->vertex_format); */
        /* getchar(); */
        return mesh;
    } else {
        // Invalid mesh filetype or it is not supported.
        load_error("invalid mesh filetype");
    }
}
#undef load_error

void init_resources_rendering(void)
{
    add_resource_type(Shader);
    add_resource_type(Mesh);
    add_resource_type(Texture);

    add_resource_type(MaterialType);
    add_resource_type(Material);
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

    if ((vertex_format & ~VERTEX_FORMAT_3 & ~VERTEX_FORMAT_C & ~VERTEX_FORMAT_U) != 0) {
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


//--------------------------------------------------------------------------------
// Mesh stuff
//--------------------------------------------------------------------------------
// Remember to add here when new vertex attributes are used!
const AttributeInfo g_attribute_info[NUM_ATTRIBUTE_TYPES] = {
    { ATTRIBUTE_TYPE_POSITION, "vPosition", GL_FLOAT, 3 },
    { ATTRIBUTE_TYPE_COLOR, "vColor", GL_FLOAT, 3 },
    { ATTRIBUTE_TYPE_NORMAL, "vNormal", GL_FLOAT, 3},
    { ATTRIBUTE_TYPE_UV, "vTexCoord", GL_FLOAT, 2},
};

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
        if ((mesh->vertex_format & (1 << i)) != 0) { // vertex format has attribute i set
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

//--------------------------------------------------------------------------------
// Materials
//--------------------------------------------------------------------------------
ResourceType MaterialType_RTID;
void *MaterialType_load(char *path)
{
    //----- Important: Some error conditions leave memory leaks. Seriously think about how to avoid this.
#define load_error(STRING)\
    { fprintf(stderr, "Error while loading MaterialType: " STRING "\n"); return NULL; }
    // Try to open the .MaterialType dictionary for querying.
    FILE *file = resource_file_open(path, ".MaterialType", "r");
    if (file == NULL) load_error("File failed to open.");
    Dictionary *dict = dictionary_read(file);
    if (dict == NULL) load_error("Failed to create dictionary for file.");
    DictQuerier *q = dict_new_querier(dict);
    if (q == NULL) load_error("Could not create querier.");
    dict_query_rules_rendering(q);
    // Now query for the basic MaterialType values.
    MaterialType material_type;
    if (!dict_query_get(q, "VertexFormat", "vertex_format", &material_type.vertex_format)) load_error("Non-existent/invalid vertex_format entry.");
    if (!dict_query_get(q, "unsigned int", "num_blocks", &material_type.num_blocks)) load_error("Non-existent/invalid num_blocks entry.");
    if (!dict_query_get(q, "unsigned int", "num_textures", &material_type.num_textures)) load_error("Non-existent/invalid num_textures entry.");
    if (material_type.num_blocks > MATERIAL_MAX_SHADER_BLOCKS) load_error("Too many shader blocks declared in num_blocks.");
    if (material_type.num_textures > MATERIAL_MAX_TEXTURES) load_error("Too many textures declared in num_textures.");

    // Attempt to find/load the Shader resources, and then link them.
    const int buf_size = 1024;
    char buf[buf_size];
    material_type.program_type = GRAPHICS_PROGRAM_VF; // Only handling vertex+fragment programs currently.
    if (!dict_get(dict, "vertex_shader", buf, buf_size)) load_error("No vertex_shader entry.");
    material_type.shaders[Vertex] = new_resource_handle(Shader, buf);
    if (!dict_get(dict, "fragment_shader", buf, buf_size)) load_error("No fragment_shader entry."); //!!!! Destroy the above resource handle.
    material_type.shaders[Fragment] = new_resource_handle(Shader, buf);

    material_type.program_id = glCreateProgram();
    glAttachShader(material_type.program_id, resource_data(Shader, material_type.shaders[Vertex])->shader_id);
    glAttachShader(material_type.program_id, resource_data(Shader, material_type.shaders[Fragment])->shader_id);
    // Link the program, and do error-checking.
    link_shader_program(material_type.program_id);
    // Detach the shaders so they can be deleted if needed.
    glDetachShader(material_type.program_id, resource_data(Shader, material_type.shaders[Vertex])->shader_id);
    glDetachShader(material_type.program_id, resource_data(Shader, material_type.shaders[Fragment])->shader_id);

    // A material instance text-file of this material type defines its textures from their names given in this material type text-file. Collate these
    // names.
    for (int i = 0; i < material_type.num_textures; i++) {
        char texture_token[32];
        sprintf(texture_token, "texture%d", i);
        if (!dict_get(dict, texture_token, buf, buf_size)) load_error("Not all declared textures have been given.");
        if (strlen(buf) > MATERIAL_MAX_TEXTURE_NAME_LENGTH) load_error("Texture name too long.");
        strncpy(material_type.texture_names[i], buf, MATERIAL_MAX_TEXTURE_NAME_LENGTH);
    }
    // Collect shader-block information and bind their backing buffers to the linked program.
    //      Possibly the material does not even need to keep information about its blocks after binding them to its program.
    //      However, for printing at least it is useful.
    //      ----
    //      Actually, it may be needed so that draw calls can trigger synchronization of relevant shader-blocks.
    //      All of them could be checked anyway, though, which may be easier.
    for (int i = 0; i < material_type.num_blocks; i++) {
        char block_token[32];
        sprintf(block_token, "block%d", i);
        if (!dict_get(dict, block_token, buf, buf_size)) load_error("Not all declared shader blocks have been given.");
        ShaderBlockID block_id = get_shader_block_id(buf);
        if (block_id < 0) load_error("Unsupported shader block.");
        material_type.shader_blocks[i] = block_id;
        // Bind the block to the linked program.
        GLuint block_index = glGetUniformBlockIndex(material_type.program_id, buf);
        if (block_index == GL_INVALID_INDEX) {
            // Hopefully the glsl compiler does not optimize away entire std140 uniform blocks.
            load_error("Could not find uniform block.");
        }
        // The block id is both index into the global block info array, and the binding point.
        glBindBufferBase(GL_UNIFORM_BUFFER, block_id, g_shader_blocks[block_id].vram_buffer_id);
    }
    // Successfully filled the MaterialType.
    MaterialType *out_material_type = (MaterialType *) calloc(1, sizeof(MaterialType));
    mem_check(out_material_type);
    memcpy(out_material_type, &material_type, sizeof(MaterialType));
    return out_material_type;
#undef load_error
}


ResourceType Material_RTID;
void *Material_load(char *path)
{
#define load_error(STRING)\
    { fprintf(stderr, "Error while loading Material: " STRING "\n"); return NULL; }
    // Try to open the .Material dictionary for querying.
    FILE *file = resource_file_open(path, ".Material", "r");
    if (file == NULL) load_error("File failed to open.");
    Dictionary *dict = dictionary_read(file);
    if (dict == NULL) load_error("Failed to create dictionary for file.");
    DictQuerier *q = dict_new_querier(dict);
    if (q == NULL) load_error("Could not create querier.");
    dict_query_rules_rendering(q);

    Material material;
    const int buf_size = 1024;
    char buf[buf_size];
    if (!dict_get(dict, "material_type", buf, buf_size)) load_error("No resource path given for the material type (no material_type entry).");
    material.material_type = new_resource_handle(MaterialType, buf);
    MaterialType *material_type = resource_data(MaterialType, material.material_type);

    // Create the texture resource handles for each texture of this material type.
    for (int i = 0; i < material_type->num_textures; i++) {
        char texture_token[MATERIAL_MAX_TEXTURE_NAME_LENGTH + 3 + 1]; // to handle texture names up to the maximum length
        sprintf(texture_token, "tx_%s", material_type->texture_names[i]);
        if (!dict_get(dict, texture_token, buf, buf_size)) load_error("Missing texture that is required for declared material type.");
        material.textures[i] = new_resource_handle(Texture, buf);
    }

    Material *out_material = (Material *) malloc(sizeof(Material));
    mem_check(out_material);
    memcpy(out_material, &material, sizeof(Material));
    return out_material;
#undef load_error
}
/* void Material_reload(ResourceHandle handle) */
/* { */
/*     // Reload the material-type's shaders. */
/*     MaterialType *material = resource_data(Material, handle); */
/*     for (int i = 0; i < NUM_SHADER_TYPES; i++) { */
/*         if ((material->program_type & (1 << i)) != 0) { */
/*             Shader_reload(material->shaders[i]); */
/*         } */
/*     } */
/*     //-----Important: Resource system needs destruction and unloading. This leaks resources. */
/*     // reload the file. */
/*     // ----- */
/* } */


void synchronize_shader_blocks(void)
{
    for (int i = 0; i < g_num_shader_blocks; i++) {
        if (!g_shader_blocks[i].dirty) continue;
        // just update the whole buffer for now. I don't even know if it is worth subdataing.
        glBindBuffer(GL_UNIFORM_BUFFER, g_shader_blocks[i].vram_buffer_id);
        glBufferSubData(GL_UNIFORM_BUFFER, (GLintptr) 0, g_shader_blocks[i].size, g_shader_blocks[i].shader_block);
        
        g_shader_blocks[i].dirty = false;
    }
}

void mesh_material_draw(Mesh *mesh, Material *material)
{
    printf("Drawing ...\n");

    MaterialType *material_type = resource_data(MaterialType, material->material_type);

    glBindVertexArray(mesh->vertex_array_id);
    glUseProgram(material_type->program_id);

    synchronize_shader_blocks();

    // Bind the textures.
    for (int i = 0; i < material_type->num_textures; i++) {
        Texture *texture = resource_data(Texture, material->textures[i]);
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, texture->texture_id);
    }

    glDrawElements(GL_TRIANGLES, 3 * mesh->num_triangles, GL_UNSIGNED_INT, (void *) 0);
}



/*
vertex_format: 3NU
vertex_shader: Shaders/test.vert
fragment_shader: Shaders/test.frag
num_blocks: 3
// block for window and time information, for interactive windowed applications
block0: StandardLoopWindow
block1: Standard3D
block2: DirectionalLights
properties: Properties
num_textures: 2
texture0: diffuse_map
texture1: normal_map
*/


void ___print_shader_block(ShaderBlockID id)
{
#define bstring(BOOLEAN) ( ( BOOLEAN ) ? "true" : "false" )
    ShaderBlockInfo *block = &g_shader_blocks[id];
    printf("name: \"%s\"\n", block->name);
    printf("id: %d\n", (int) block->id);
    printf("dirty?: %s\n", bstring(block->dirty));
    printf("size: %lu\n", block->size);
    printf("vram_buffer_id: %u\n", block->vram_buffer_id);
    printf("DIRTY FLAGS\n");
    for (int i = 0; i < block->size; i++) {
        printf("%d: %s\n", i, bstring(block->dirty_flags[i]));
    }
#undef bstring
}



void ___add_shader_block(ShaderBlockID *id_pointer, size_t size, char *name)
{
    if (g_num_shader_blocks >= MAX_NUM_SHADER_BLOCKS) {
        fprintf(stderr, ERROR_ALERT "Too many shader blocks have been created. The maximum is set to %d, but this can be changed.\n", MAX_NUM_SHADER_BLOCKS);
        exit(EXIT_FAILURE);
    }
    ShaderBlockInfo *new_block = &g_shader_blocks[g_num_shader_blocks];
    *id_pointer = (ShaderBlockID) g_num_shader_blocks; // Setting the ID to the index in the global info array.

    // Fill the type information.
    new_block->id = *id_pointer;
    new_block->size = size;
    new_block->name = (char *) malloc(sizeof(char) * (strlen(name) + 1));
    mem_check(new_block->name);
    strcpy(new_block->name, name);
    new_block->shader_block = calloc(1, size);
    mem_check(new_block);
    // This flag, if false, allows the checking of every entry to be skipped when synchronizing to the backing buffer.
    new_block->dirty = true;

    // Create a bool array of dirty flags for each entry in the flattened shdaer-block struct
    // (with the padding, but it shouldn't matter.)
    // IMPORTANT: The synchronizer doesn't know types. So, dirty flags must be set for the whole width of an updated entry.
    new_block->dirty_flags = (bool *) malloc(sizeof(bool) * size);
    mem_check(new_block->dirty_flags);
    for (int i = 0; i < size; i++) new_block->dirty_flags[i] = true;

    // Create a VRAM buffer to back the block.
    GLuint ubo;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_DYNAMIC_DRAW); // Which buffer usage token should be used? (p96 OGLPG8e)
    new_block->vram_buffer_id = ubo;

    g_num_shader_blocks ++;
}

void ___set_uniform_float(ShaderBlockID id, float *entry_address, float val)
{
    g_shader_blocks[id].dirty = true;
    size_t offset = ((bool *) entry_address) - ((bool *) g_shader_blocks[id].shader_block);
    for (int i = 0; i < sizeof(float); i++) g_shader_blocks[id].dirty_flags[offset + i] = true;
    *entry_address = val;
}

ShaderBlockID get_shader_block_id(char *name)
{
    for (int i = 0; i < g_num_shader_blocks; i++) {
        if (strcmp(name, g_shader_blocks[i].name) == 0) {
            return g_shader_blocks[i].id;
        }
    }
    return -1;
}
