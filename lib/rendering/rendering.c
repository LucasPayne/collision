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
#include <math.h>
#include "helper_definitions.h"
#include "resources.h"
#include "ply.h" //----------Remove this dependency.
#include "rendering.h"
#include "dictionary.h"

//----only here while the querying/parsing stuff for matrices is here
#include "matrix_mathematics.h"

int g_num_shader_blocks = 0;
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
            casemap('I', (1 << ATTRIBUTE_TYPE_INDEX));
            default:
                return VERTEX_FORMAT_NONE;
        }
    }
    return vertex_format;
#undef casemap
}


static bool query_val_int(char *string, void *data)
{
    int i;
    if (sscanf(string, "%d", &i) == EOF) return false;
    memcpy(data, &i, sizeof(int));
    return true;
}
static bool query_val_bool(char *string, void *data)
{
    bool b;
    if (strcmp(string, "true") == 0) b = true;
    else if (strcmp(string, "false") == 0) b = false;
    else return false;
    memcpy(data, &b, sizeof(bool));
    return true;
}
static bool query_val_float(char *string, void *data)
{
    float f;
    if (sscanf(string, "%f", &f) == EOF) return false;
    memcpy(data, &f, sizeof(float));
    return true;
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
static bool query_val_vec4(char *string, void *data)
{
    // format: 0, 1, 2, 3
    // [0]
    // [1]
    // [2]
    // [3]
    char *p = string;
    vec4 v;
    int chars_read;
    for (int i = 0; i < 4; i++) {
        if (sscanf(p, (i < 3) ? "%f%n," : "%f%n", &v.vals[i], &chars_read) == EOF) return false;
        p += chars_read + 1;
    }
    memcpy(data, &v, sizeof(vec4));
    return true;
}
static bool query_val_mat4x4(char *string, void *data)
{
    // format: 0 1 2 3 ; 4 5 6 7 ; 8 9 10 11 ; 12 13 14 15
    // [ 0 4 8 12  ]
    // [ 1 5 9 13  ]
    // [ 2 6 10 14 ]
    // [ 3 7 11 15 ]
    Matrix4x4f matrix;
    char *p = string;
#define eat() { while (isspace(*p)) { p++; } }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            eat();
            int num_chars;
            if (sscanf(p, "%f%n", &matrix.vals[4*i + j], &num_chars) == EOF) {
                return false;
            }
            p += num_chars;
            eat();
            if (j < 3 && *p++ != ',') {
                return false;
            } else if (i < 3 && j == 3 && *p++ != ';') {
                return false;
            }
        }
    }
    memcpy(data, &matrix, sizeof(Matrix4x4f));
    return true;
}
void dict_query_rules_rendering(DictQuerier *q)
{
    dict_query_rule_add(q, "VertexFormat", query_val_VertexFormat);
    
    // move to dictionary's basic types 
    dict_query_rule_add(q, "int", query_val_int);
    dict_query_rule_add(q, "unsigned int", query_val_unsigned_int);
    dict_query_rule_add(q, "float", query_val_float);
    dict_query_rule_add(q, "bool", query_val_bool);

    // move to matrix mathematics' types
    dict_query_rule_add(q, "mat4x4", query_val_mat4x4);
    dict_query_rule_add(q, "vec4", query_val_vec4);
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
        shader_type = Geom;
    } else {
        return NULL;
    }
    GLenum gl_shader_type = gl_shader_type(shader_type);
    if (gl_shader_type == 0) return NULL;
    GLuint shader_id = glCreateShader(gl_shader_type);
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
    Shader *shader = resource_data(Shader, handle);
    char shader_path_buffer[1024];
    if (!resource_file_path(handle.data.path, "", shader_path_buffer, 1024)) return false;
    GLuint test_id = glCreateShader(gl_shader_type(shader->shader_type));
    if (!load_and_compile_shader(test_id, shader_path_buffer)) return false;
    // The compilation was successful on the test ID, so give this to the resource handle.
    shader->shader_id = test_id;
    return true;
}


ResourceType Geometry_RTID;
#define load_error(STRING)\
{\
    fprintf(stderr, ERROR_ALERT "Error loading goemetry: %s\n", ( STRING ));\
    exit(EXIT_FAILURE);\
}
void *Geometry_load(char *path)
{
    /* File-backed Geometry:
     *  - Triangle mesh described in a .Mesh file
     *      (currently this just has the vertex format and mesh data filetype (PLY))
     */
    FILE *file = resource_file_open(path, ".Mesh", "r");
    if (file == NULL) load_error("could not find a .Mesh file");
    Dictionary *dict = dictionary_read(file);
    if (dict == NULL) load_error("dictionary can't be made");
    const int buf_size = 512;
    char buf[buf_size];
    if (!dict_get(dict, "vertex_format", buf, buf_size)) load_error("vertex_format cannot be found");
    VertexFormat vertex_format = string_to_VertexFormat(buf);
    if (vertex_format == VERTEX_FORMAT_NONE) load_error("vertex_format is invalid");
    if ((vertex_format & VERTEX_FORMAT_3) == 0) load_error("vertex_format must contain a position attribute (symbol: 3).");
    if (!dict_get(dict, "filetype", buf, buf_size)) load_error("filetype cannot be found");


    MeshData mesh_data = {0};
    Geometry geometry;
    if (strncmp(buf, "ply", buf_size) == 0) {
        // Loading the mesh from a PLY file.
        FILE *ply_file = resource_file_open(path, ".ply", "r");
        if (ply_file == NULL) load_error("cannot open resource PLY file");
        load_mesh_ply(&mesh_data, vertex_format, ply_file);
        geometry = upload_mesh(&mesh_data);
    } else {
        // Invalid mesh filetype or it is not supported.
        load_error("invalid mesh filetype");
    }
    // DESTROY THE MESH DATA !!!!
    for (int i = 0; i < NUM_ATTRIBUTE_TYPES; i++) {
        if (mesh_data.attribute_data[i] != NULL) free(mesh_data.attribute_data[i]);
    }
    if (mesh_data.triangles != NULL) free(mesh_data.triangles);

    Geometry *geometry_out = (Geometry *) calloc(1, sizeof(Geometry));
    mem_check(geometry_out);
    memcpy(geometry_out, &geometry, sizeof(Geometry));
    return geometry_out;
}
#undef load_error

void init_resources_rendering(void)
{
    add_resource_type_no_unload(Shader);
    add_resource_type_no_unload(Geometry);
    add_resource_type_no_unload(Texture);

    add_resource_type_no_unload(MaterialType);
    add_resource_type_no_unload(Material);
}

//--------------------------------------------------------------------------------
// Loaders
//--------------------------------------------------------------------------------
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
    { ATTRIBUTE_TYPE_INDEX, "vIndex", GL_UNSIGNED_INT, 1},
};

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

//--------------------------------------------------------------------------------
// Materials
//--------------------------------------------------------------------------------
ResourceType MaterialType_RTID;
void *MaterialType_load(char *path)
{
    /* Parse the material type information from its text file. This is the only way to define new types of materials.
     * This includes:
     *      - Collecting metadata such as vertex format and which types of shaders are in the linked program.
     *      - Finding/loading the shaders required, then linking them into a program.
     *      - Collecting names and texture units for the textures each material instance should provide.
     *      - Collecting shader block information, shader blocks being global shared blocks the application can easily interact with,
     *          which will be synchronized to vram buffers for access by this material's shaders.
     *      - Collecting material property information, the properties being available (if any) in the shaders with a block
     *          "MaterialProperties". This is a regular shader block, but is used in a specialized way to allow per-material-instance
     *          properties, such as specular parameters for a Phong-lit material.
     *
     * After loading a material type, it is prepared for material instancing (materials which hold this as its material type.)
     *
     * Text-file format:
     *      vertex_format: <format characters, e.g. 3NU>
     *      vertex_shader: <resource path>
     *      fragment_shader: <resource path>
     *          etc. for other shaders (if supported)
     *      num_blocks: <int>, the number of shader blocks used
     *      block{i}: <string>, the name of the shader block
     *          ... Note: if the material type has properties, some block{i} will be "MaterialProperties".
     *      num_textures: <int>
     *      texture{i}: <string>, the name of this texture
     *          ...
     */
    printf("Loading material type from path %s\n", path);

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
    MaterialType material_type = {0};
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
    // Attach the shaders. If they aren't loaded resources, then this loads and tries to compile them.
    glAttachShader(material_type.program_id, resource_data(Shader, material_type.shaders[Vertex])->shader_id);
    glAttachShader(material_type.program_id, resource_data(Shader, material_type.shaders[Fragment])->shader_id);
    // Link the program, and do error-checking.
    link_shader_program(material_type.program_id);
    // Detach the shaders so they can be deleted if needed.
    glDetachShader(material_type.program_id, resource_data(Shader, material_type.shaders[Vertex])->shader_id);
    glDetachShader(material_type.program_id, resource_data(Shader, material_type.shaders[Fragment])->shader_id);

    // A file-backed material instance of this material-type defines its textures from their names given in this material type text-file. Collate these
    // names, and attach texture{i}'s declared texture name to texture unit GL_TEXTURE{i}.
    glUseProgram(material_type.program_id); // use the program so sampler uniforms can be uploaded.
    for (int i = 0; i < material_type.num_textures; i++) {
        char texture_token[32];
        sprintf(texture_token, "texture%d", i);
        if (!dict_get(dict, texture_token, buf, buf_size)) load_error("Not all declared textures have been given.");
        if (strlen(buf) > MATERIAL_MAX_TEXTURE_NAME_LENGTH) load_error("Texture name too long.");
        strncpy(material_type.texture_names[i], buf, MATERIAL_MAX_TEXTURE_NAME_LENGTH);
        // Bind this texture in the program to the binding point.
        GLint texture_location = glGetUniformLocation(material_type.program_id, buf);
        if (texture_location < 0) {
            // ----Since a sampler in glsl is a loose uniform variable, maybe it could be optimized out. -1 being passed to texture functions
            // fails silently, so it might be fine to not give a load error here.
            load_error("Could not find sampler in linked program.");
        }
        glUniform1i(texture_location, i);
    }
    glUseProgram(0);

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
            // (It appears that it doesn't.)
            load_error("Could not find uniform block.");
        }
        glUniformBlockBinding(material_type.program_id, block_index, block_id);
        // The block id is both index into the global block info array, and the binding point.
        glBindBufferBase(GL_UNIFORM_BUFFER, block_id, g_shader_blocks[block_id].vram_buffer_id);
    }

    // There is a special shader block, MaterialProperties, which is the per-material-instance interface to the parameters
    // of this material type.
    // Collect material-properties information and store this in the info struct array of the material type.
    bool has_property_block = false;
    for (int i = 0; i < material_type.num_blocks; i++) {
        if (material_type.shader_blocks[i] == ShaderBlockID_MaterialProperties) {
            has_property_block = true;
            break;
        }
    }
    if (has_property_block) {
        GLuint mp_block_index = glGetUniformBlockIndex(material_type.program_id, "MaterialProperties");
        if (mp_block_index == GL_INVALID_INDEX) {
            fprintf(stderr, ERROR_ALERT "Something went wrong. MaterialProperties block index cannot be found for material type which apparently uses one.\n");
            exit(EXIT_FAILURE);
        }
        const int max_num_properties = 512;
        glGetActiveUniformBlockiv(material_type.program_id, mp_block_index, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &material_type.num_properties);
        if (material_type.num_properties > max_num_properties) {
            fprintf(stderr, ERROR_ALERT "Too many entries defined in a MaterialProperties block. The maximum is set to %d, but this can be changed.\n", max_num_properties);
            exit(EXIT_FAILURE);
        }
        GLuint uniform_indices[max_num_properties];
        glGetActiveUniformBlockiv(material_type.program_id, mp_block_index, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, uniform_indices);

        glGetActiveUniformBlockiv(material_type.program_id, mp_block_index, GL_UNIFORM_BLOCK_DATA_SIZE, &material_type.properties_size);
        if (material_type.properties_size > g_shader_blocks[ShaderBlockID_MaterialProperties].size) {
            fprintf(stderr, ERROR_ALERT "MaterialProperties block encountered which is too large. The maximum block size is %zu, which can be changed.\n", g_shader_blocks[ShaderBlockID_MaterialProperties].size);
            exit(EXIT_FAILURE);
        }

        // Collect information about the entries of the MaterialProperties uniform block.
        for (int i = 0; i < material_type.num_properties; i++) {
            MaterialPropertyInfo info = {0};
            info.location = uniform_indices[i];
            int name_length;
            glGetActiveUniformName(material_type.program_id, uniform_indices[i], MATERIAL_MAX_PROPERTY_NAME_LENGTH + 1, &name_length, info.name); //+1?
            if (name_length > MATERIAL_MAX_PROPERTY_NAME_LENGTH) {
                fprintf(stderr, ERROR_ALERT "Entry name \"%s\" in MaterialProperties uniform block is too long. The maximum name length is %d.\n", info.name, MATERIAL_MAX_PROPERTY_NAME_LENGTH);
                exit(EXIT_FAILURE);
            }
            info.name[name_length] = '\0';
            glGetActiveUniformsiv(material_type.program_id, 1, &uniform_indices[i], GL_UNIFORM_TYPE, (GLint *) &info.type);
            glGetActiveUniformsiv(material_type.program_id, 1, &uniform_indices[i], GL_UNIFORM_OFFSET, &info.offset);
            material_type.property_infos[i] = info;
        }
    } else {
        material_type.properties_size = 0;
        material_type.num_properties = 0;
    }
    printf("Finished loading material type from path %s\n", path);
    print_material_type(&material_type);
    
    // Successfully filled the MaterialType.
    MaterialType *out_material_type = (MaterialType *) calloc(1, sizeof(MaterialType));
    mem_check(out_material_type);
    memcpy(out_material_type, &material_type, sizeof(MaterialType));
    return out_material_type;
#undef load_error
}

void print_material_type(MaterialType *mt)
{
    printf("MaterialType printout\n");
    printf("=====================\n");
    print_vertex_format(mt->vertex_format);
    printf("num_blocks: %d\n", mt->num_blocks);
    printf("---Shader blocks-----\n");
    for (int i = 0; i < mt->num_blocks; i++) {
        ___print_shader_block(mt->shader_blocks[i]);
    }
    printf("---------------------\n");
    printf("num_textures: %d\n", mt->num_textures);
    printf("---Textures----------\n");
    for (int i = 0; i < mt->num_textures; i++) {
        printf("texture%d: %s\n", i, mt->texture_names[i]);
    }
    printf("---------------------\n");
    printf("properties_size: %zu\n", mt->properties_size);
    printf("num_properties: %d\n", mt->num_properties);
    printf("---Properties--------\n");
    for (int i = 0; i < mt->num_properties; i++) {
        printf("property name: %s\n", mt->property_infos[i].name);
        printf("property location: %d\n", mt->property_infos[i].location);
        printf("property offset: %zu\n", mt->property_infos[i].offset);
        printf("property type: %u\n", mt->property_infos[i].type);
    }
    printf("---------------------\n");
    printf("program_id: %u\n", mt->program_id);
}


ResourceType Material_RTID;
void *Material_load(char *path)
{
    /* For a material instance, file-backing is just an option. It may be useful to
     * ignore the fact that it is a shareable resource and just define new materials in-line in code,
     * but still be able to back a collection of material instance descriptions to be loaded, e.g. for standard flat colours/dashed lines
     * for a debug-line rendering system.
     *
     * This is in contrast to the material type, which must be file-backed, since it is a complicated object. All that is needed for a material instance
     * is the material-type, and textures (their resource paths for file-backed textures only) and properties.
     *
     * Text-file format:
     *      material_type: <resource path>
     *      mp_{name}: <type of this material property>, these entries define the material properties.
     *          ...
     *      tx_{name}: <resource path>, the resource path for the file-backed texture to bind to whatever unit the material type binds {name} to.
     *          ...
     */
    printf("Loading a file-backed material instance from path %s\n", path);

#define load_error(STRING)\
    { fprintf(stderr, "Error while loading Material: " STRING "\n"); return NULL; }
    // Try to open the .Material file for querying.
    FILE *file = resource_file_open(path, ".Material", "r");
    if (file == NULL) load_error("File failed to open.");
    Dictionary *dict = dictionary_read(file);
    if (dict == NULL) load_error("Failed to create dictionary for file.");
    DictQuerier *q = dict_new_querier(dict);
    if (q == NULL) load_error("Could not create querier.");
    dict_query_rules_rendering(q);

    const int buf_size = 1024;
    char buf[buf_size];
    if (!dict_get(dict, "material_type", buf, buf_size)) load_error("No resource path given for the material type (no material_type entry).");
    Material material = {0};
    material.material_type = new_resource_handle(MaterialType, buf);

    MaterialType *material_type = resource_data(MaterialType, material.material_type);

    // Create the texture resource handles for each texture of this material type.
    for (int i = 0; i < material_type->num_textures; i++) {
        char texture_token[MATERIAL_MAX_TEXTURE_NAME_LENGTH + 3 + 1]; // to handle texture names up to the maximum length
        sprintf(texture_token, "tx_%s", material_type->texture_names[i]);
        if (!dict_get(dict, texture_token, buf, buf_size)) load_error("Missing texture that is required for declared material type.");
        material.textures[i] = new_resource_handle(Texture, buf);
    }
    
    // If there is a MaterialProperties block in this material's type, find its properties in the text file.
    if (material_type->num_properties > 0 && material_type->properties_size > 0) {
        // This material type has a MaterialProperties block.
        // Allocate memory for the properties
        material.properties = calloc(1, material_type->properties_size);
        mem_check(material.properties);
        for (int i = 0; i < material_type->num_properties; i++) {
            MaterialPropertyInfo *info = &material_type->property_infos[i];
            // Query for the token "mp_<material property name>" for each material property, and look up the property info
            // so this token as a key into the dictionary can have its value parsed as the correct type into the correct offset
            // location in the properties data block in the material.
            char mp_token[3 + MATERIAL_MAX_PROPERTY_NAME_LENGTH + 1] = "mp_";
            strcpy(mp_token + 3, info->name);
            #define parse(TYPE)\
            {\
                printf("getting of type %s\n", ( TYPE ));\
                if (!dict_query_get(q, ( TYPE ), mp_token, material.properties + info->offset)) {\
                    fprintf(stderr, ERROR_ALERT "Failed to parse entry in dictionary \"%.500s\" as type \"" TYPE "\".\n", mp_token);\
                    exit(EXIT_FAILURE);\
                }\
            }
            switch(info->type) {
                case GL_FLOAT: parse("float"); break;
                case GL_BOOL: parse("bool"); break;
                case GL_FLOAT_VEC4: parse("vec4"); break;
                case GL_INT: parse("int"); break;
                case GL_UNSIGNED_INT: parse("unsigned int"); break;
                default:
                    fprintf(stderr, ERROR_ALERT "Attempted to parse a MaterialProperties block from a text file for a material type\
which has an entry with unsupported type (it cannot be deserialized from the text file).\n");
                    exit(EXIT_FAILURE);
            }
            #undef parse
        }
    } else {
        // If there is no material properties block, the size is zero. So, do not allocate memory for properties.
        material.properties = NULL;
    }
    printf("Finished loading file-backed material instance from path %s\n", path);

    Material *out_material = (Material *) malloc(sizeof(Material));
    mem_check(out_material);
    memcpy(out_material, &material, sizeof(Material));
    return out_material;
#undef load_error
}

static void ___material_set_property(Material *material, char *property_name, void *data, size_t size)
{
    // note: This type of property-setting is expensive (requiring a string search) but convenient. If material properties are configured
    //       a lot (e.g. some changing colour over time) and that becomes a problem, consider something like a macro to cast the MaterialProperties block
    //       to edit it like other blocks, with an actual struct giving the offsets for properties.
    MaterialType *mt = resource_data(MaterialType, material->material_type);

    // Check that this material has properties that can be set.
    if (mt->properties_size <= 0 || mt->num_properties == 0) {
        fprintf(stderr, ERROR_ALERT "Attempted to set a property in a material with material-type that has no properties block.\n");
        exit(EXIT_FAILURE);
    }
    // Make sure the properties are initialized. This is so that a material can be created in-line (zero-initialized), then properties edited easily without
    // having to initialize the properties data explicitly.
    if (material->properties == NULL) {
        material->properties = calloc(1, mt->properties_size);
        mem_check(material->properties);
    }
    for (int i = 0; i < mt->num_properties; i++) {
        if (strcmp(mt->property_infos[i].name, property_name) == 0) {
            // Found the property name.
            memcpy(material->properties + mt->property_infos[i].offset, data, size);
            return;
        }
    }
    fprintf(stderr, ERROR_ALERT "Attempted to set non-existent property \"%s\" in material instance.\n", property_name);
    exit(EXIT_FAILURE);
}
void material_set_property_float(Material *material, char *property_name, float val)
{
    ___material_set_property(material, property_name, (void *) &val, sizeof(float));
}
void material_set_property_vec4(Material *material, char *property_name, vec4 v)
{
    ___material_set_property(material, property_name, (void *) &v, sizeof(vec4));
}



void synchronize_shader_blocks(void)
{
    for (int i = 0; i < g_num_shader_blocks; i++) {
        if (!g_shader_blocks[i].dirty) continue;
        /* printf("Synchronizing %s\n", g_shader_blocks[i].name); */
        /* ___print_shader_block(i); */
        // just update the whole buffer for now. I don't even know if it is worth subdataing.
        glBindBuffer(GL_UNIFORM_BUFFER, g_shader_blocks[i].vram_buffer_id);
        /* printf("size: %lu\n", g_shader_blocks[i].size); */
        /* glBufferData(GL_UNIFORM_BUFFER, g_shader_blocks[i].size, g_shader_blocks[i].shader_block, GL_DYNAMIC_DRAW); */
        glBufferSubData(GL_UNIFORM_BUFFER, (GLintptr) 0, g_shader_blocks[i].size, g_shader_blocks[i].shader_block);
        
        g_shader_blocks[i].dirty = false;
    }
}

void ___print_shader_block(ShaderBlockID id)
{
#define bstring(BOOLEAN) ( ( BOOLEAN ) ? "true" : "false" )
    ShaderBlockInfo *block = &g_shader_blocks[id];
    printf("name: \"%s\"\n", block->name);
    printf("id: %d\n", (int) block->id);
    printf("dirty?: %s\n", bstring(block->dirty));
    printf("size: %lu\n", block->size);
    printf("vram_buffer_id: %u\n", block->vram_buffer_id);
    /* printf("DIRTY FLAGS\n"); */
    /* for (int i = 0; i < block->size; i++) { */
    /*     printf("%d: %s\n", i, bstring(block->dirty_flags[i])); */
    /* } */
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


void ___set_uniform_mat4x4(ShaderBlockID id, float *entry_address, float *vals)
{
    g_shader_blocks[id].dirty = true;
    size_t offset = ((bool *) entry_address) - ((bool *) g_shader_blocks[id].shader_block);
    for (int i = 0; i < 16*sizeof(float); i++) g_shader_blocks[id].dirty_flags[offset + i] = true;
    /* for (int i = 0; i < 16; i++) printf("%.2f ", vals[i]); */
    /* getchar(); */
    memcpy(entry_address, vals, 16*sizeof(float));
    /* for (int i = 0; i < 16; i++) printf("%.2f ", entry_address[i]); */
    /* getchar(); */
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

// working on currently
//---------------------

static bool g_gm_initialized = false;
static bool g_gm_specifying = false;

static Geometry g_gm_id;

static uint32_t *g_gm_index_buffer;
static int g_gm_index_count;

static void *g_gm_attribute_buffers[NUM_ATTRIBUTE_TYPES];
static size_t g_gm_attribute_positions[NUM_ATTRIBUTE_TYPES];
static int g_gm_attribute_counts[NUM_ATTRIBUTE_TYPES];

#define gm_size_error()\
{\
    fprintf(stderr, ERROR_ALERT "Attempted to specify too-large geometry.\n");\
    exit(EXIT_FAILURE);\
}
#define gm_attribute_error()\
{\
    fprintf(stderr, ERROR_ALERT "Attribute given in geometry upload which does not match the declared vertex format.\n");\
    exit(EXIT_FAILURE);\
}
#define gm_init_check()\
{\
    if (!g_gm_initialized) {\
        gm_init();\
    }\
}

static void gm_reset(void)
{
    memset(&g_gm_id, 0, sizeof(Geometry));
    for (int i = 0; i < NUM_ATTRIBUTE_TYPES; i++) {
        g_gm_attribute_positions[i] = 0;
        g_gm_attribute_counts[i] = 0;
    }
    g_gm_index_count = 0;
}
static void gm_init(void)
{
    for (int i = 0; i < NUM_ATTRIBUTE_TYPES; i++) {
        g_gm_attribute_buffers[i] = calloc(1, GM_ATTRIBUTE_BUFFER_SIZE);
        mem_check(g_gm_attribute_buffers[i]);
        g_gm_attribute_positions[i] = 0;
    }
    g_gm_index_buffer = calloc(1, GM_INDEX_BUFFER_SIZE);
    mem_check(g_gm_index_buffer);
    g_gm_initialized = true;
}


#define attribute_check(SIZE,TYPE)\
{\
    if ((g_gm_id.vertex_format & (1 << attribute_type)) == 0) gm_attribute_error();\
    if (g_attribute_info[attribute_type].gl_size != ( SIZE ) || g_attribute_info[attribute_type].gl_type != ( TYPE )) {\
        fprintf(stderr, ERROR_ALERT "Type mismatch when specifying attribute value. Make sure that the right attribute_* function is used.\n");\
        exit(EXIT_FAILURE);\
    }\
}
uint32_t attribute_1u(AttributeType attribute_type, uint32_t u)
{
    attribute_check(1, GL_UNSIGNED_INT);
    size_t pos = g_gm_attribute_positions[attribute_type];
    if (pos + sizeof(uint32_t) > GM_ATTRIBUTE_BUFFER_SIZE) gm_size_error();
    memcpy(g_gm_attribute_buffers[attribute_type] + pos, &u, sizeof(uint32_t));
    // Returns the index, for convenience in defining index lists.
    return g_gm_attribute_counts[attribute_type] ++;
}
uint32_t attribute_3f(AttributeType attribute_type, float a, float b, float c)
{
    attribute_check(3, GL_FLOAT);

    size_t pos = g_gm_attribute_positions[attribute_type];
    if (pos + 3*sizeof(float) > GM_ATTRIBUTE_BUFFER_SIZE) gm_size_error();
    memcpy(g_gm_attribute_buffers[attribute_type] + pos, &a, sizeof(float));
    memcpy(g_gm_attribute_buffers[attribute_type] + pos + sizeof(float), &b, sizeof(float));
    memcpy(g_gm_attribute_buffers[attribute_type] + pos + 2*sizeof(float), &c, sizeof(float));
    g_gm_attribute_positions[attribute_type] += 3*sizeof(float);

    // Returns the index, for convenience in defining index lists.
    return g_gm_attribute_counts[attribute_type] ++;
}
#undef attribute_check
void attribute_buf(AttributeType attribute_type, void *buf, int count)
{
    /* Specify the attribute data from a buffer. This will be faster than streaming through, if you already
     * have the data in the format needed for the attribute.
     */
    size_t width = g_attribute_info[attribute_type].gl_size * gl_type_size(g_attribute_info[attribute_type].gl_type); // the width of a single one of these attributes.
    size_t pos = g_gm_attribute_positions[attribute_type];
    if ((g_gm_id.vertex_format & (1 << attribute_type)) == 0) gm_attribute_error();
    if (pos + width * count > GM_ATTRIBUTE_BUFFER_SIZE) gm_size_error();
    memcpy(g_gm_attribute_buffers[attribute_type] + pos, buf, width * count);
    g_gm_attribute_positions[attribute_type] += width * count;
    g_gm_attribute_counts[attribute_type] += count;
}


void gm_index(uint32_t index)
{
    if (g_gm_index_count >= GM_INDEX_BUFFER_SIZE) gm_size_error();

    g_gm_index_buffer[g_gm_index_count] = index;
    g_gm_index_count ++;
}
void gm_index_buf(uint32_t *indices, int count)
{
    if (g_gm_index_count + count > GM_INDEX_BUFFER_SIZE) gm_size_error();
    memcpy(g_gm_index_buffer + g_gm_index_count, indices, count * sizeof(uint32_t));
    g_gm_index_count += count;
}


void gm_triangles(VertexFormat vertex_format)
{
    gm_init_check();
    gm_reset();
    g_gm_specifying = true;
    g_gm_id.vertex_format = vertex_format;
    g_gm_id.primitive_type = Triangles;
    g_gm_id.is_indexed = true;
    g_gm_id.dynamic = true;
}
void gm_lines(VertexFormat vertex_format)
{
    gm_init_check();
    gm_reset();
    g_gm_specifying = true;
    g_gm_id.vertex_format = vertex_format;
    g_gm_id.primitive_type = Lines;
    g_gm_id.is_indexed = false;
    g_gm_id.dynamic = true;
}

Geometry gm_done(void)
{
    if (!g_gm_specifying) {
        fprintf(stderr, ERROR_ALERT "Cannot call gm_done when no geometry is being specified.\n");
        exit(EXIT_FAILURE);
    }
    // Check that all of the relevant attributes have had the same number of values specified.
    int last_count = -1;
    for (int i = 0; i < NUM_ATTRIBUTE_TYPES; i++) {
        if (g_gm_id.vertex_format & (1 << i)) {
            if (last_count != -1 && last_count != g_gm_attribute_counts[i]) {
                fprintf(stderr, ERROR_ALERT "Unequal numbers of vertex attributes have been specified in a geometry upload. %d != %d.\n", last_count, g_gm_attribute_counts[i]); 
                exit(EXIT_FAILURE);
            }
            last_count = g_gm_attribute_counts[i];
        }
    }

    glGenVertexArrays(1, &g_gm_id.vao_id);
    glBindVertexArray(g_gm_id.vao_id);

    for (int i = 0; i < NUM_ATTRIBUTE_TYPES; i++) {
        if (g_gm_id.vertex_format & (1 << i)) {
            glGenBuffers(1, &g_gm_id.buffer_ids[i]);
            glBindBuffer(GL_ARRAY_BUFFER, g_gm_id.buffer_ids[i]);
            glBufferData(GL_ARRAY_BUFFER, g_gm_attribute_positions[i], g_gm_attribute_buffers[i], g_gm_id.dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
            glVertexAttribPointer(i,
                                  g_attribute_info[i].gl_size,
                                  g_attribute_info[i].gl_type,
                                  GL_FALSE, // not normalized
                                  0, // no stride
                                  (void *) 0);
            glEnableVertexAttribArray(i);
        }
    }

    if (g_gm_id.is_indexed) {
        glGenBuffers(1, &g_gm_id.indices_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_gm_id.indices_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t)*g_gm_index_count, g_gm_index_buffer, g_gm_id.dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    }

    g_gm_specifying = false;

    //---- should have this kept in the struct already. Also, change the name from "ID", to maybe just Geometry.
    g_gm_id.num_indices = g_gm_index_count;
    g_gm_id.num_vertices = last_count;

    return g_gm_id;
}


void gm_draw(Geometry geometry, Material *material)
{
    MaterialType *mt = resource_data(MaterialType, material->material_type);
    glUseProgram(mt->program_id);
    // Bind the textures
    for (int i = 0; i < mt->num_textures; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, resource_data(Texture, material->textures[i])->texture_id);
    }
    // Prepare the material properties, if there are any.
    if (material->properties != NULL) {
        memcpy(g_shader_blocks[ShaderBlockID_MaterialProperties].shader_block, material->properties, mt->properties_size);
        g_shader_blocks[ShaderBlockID_MaterialProperties].dirty = true; //setting this explicitly since this is not using the macro syntax for accessing entries of the block.
    }
    synchronize_shader_blocks();

    //---- do this map elsewhere.
    GLenum gl_primitive_type;
    switch(geometry.primitive_type) {
        case Triangles: gl_primitive_type = GL_TRIANGLES; break;
        case Lines: gl_primitive_type = GL_LINES; break;
        default:
            fprintf(stderr, ERROR_ALERT "Have not accounted for the primitive type of a piece of geometry passed into gm_draw.\n");
            exit(EXIT_FAILURE);
    }

    glBindVertexArray(geometry.vao_id);
    if (geometry.is_indexed) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.indices_id);
        glDrawElements(gl_primitive_type, geometry.num_indices, GL_UNSIGNED_INT, (void *) 0);
    } else {
        glDrawArrays(gl_primitive_type, 0, geometry.num_vertices);
    }
}

void gm_free(Geometry geometry)
{
    // --- Since this is called for each transient draw, maybe it would be better
    // --- to just allocate an ID for every attribute type, and free them all at once, to reduce
    // --- the number of API calls.
    if (geometry.is_indexed) {
        glDeleteBuffers(1, &geometry.indices_id);
    }
    for (int i = 0; i < NUM_ATTRIBUTE_TYPES; i++) {
        if (geometry.vertex_format & (1 << i)) {
            glDeleteBuffers(1, &geometry.buffer_ids[i]);
        }
    }
}

#undef gm_size_error
