/*--------------------------------------------------------------------------------
    Materials component of the rendering module.

    - The resources MaterialType and Material
        - Loading a material-type definition from a file.
        - Loading the properties/texture-paths for a material from a file.
    - The "shader block" mechanism (shared buffers materials can subscribe to).
        - Direct access to shader block entries (such as setting an mvp_matrix or the time) with a convenient syntax.
        - The special shader block "MaterialProperties" which provides per-instance configuration if relevant to a material-type.
            - Configuring material properties per-instance by string lookup.
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
#include "rendering.h"
#include "dictionary.h"

int g_num_shader_blocks = 0;
ShaderBlockInfo g_shader_blocks[MAX_NUM_SHADER_BLOCKS];

ResourceType MaterialType_RTID;
/*--------------------------------------------------------------------------------
Parse the material-type information from its text file. This is the only way to define new types of materials.
This includes:
     - Collecting metadata such as vertex format and which types of shaders are in the linked program.
     - Finding/loading the shaders required, then linking them into a program.
     - Collecting names and texture units for the textures each material instance should provide.
     - Collecting shader block information, shader blocks being global shared blocks the application can easily interact with,
         which will be synchronized to vram buffers for access by this material's shaders.
     - Collecting material property information, the properties being available (if any) in the shaders with a block
         "MaterialProperties". This is a regular shader block, but is used in a specialized way to allow per-material-instance
         properties, such as specular parameters for a Phong-lit material.

After loading a material type, it is prepared for material instancing (materials which hold this as its material type.)

Text-file format:
     vertex_format: <format characters, e.g. 3NU>
     vertex_shader: <resource path>
     fragment_shader: <resource path>
         etc. for other shaders (if supported)
     num_blocks: <int>, the number of shader blocks used
     block{i}: <string>, the name of the shader block
         ... Note: if the material type has properties, some block{i} will be "MaterialProperties".
     num_textures: <int>
     texture{i}: <string>, the name of this texture
         ...
--------------------------------------------------------------------------------*/
void *MaterialType_load(char *path)
{
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
/*--------------------------------------------------------------------------------
For a material instance, file-backing is just an option. It may be useful to
ignore the fact that it is a shareable resource and just define new materials in-line in code,
but still be able to back a collection of material instance descriptions to be loaded, e.g. for standard flat colours/dashed lines
for a debug-line rendering system.

This is in contrast to the material type, which must be file-backed, since it is a complicated object. All that is needed for a material instance
is the material type, and textures (their resource paths for file-backed textures only) and properties.

Text-file format:
     material_type: <resource path>
     mp_{name}: <type of this material property>, these entries define the material properties.
         ...
     tx_{name}: <resource path>, the resource path for the file-backed texture to bind to whatever unit the material type binds {name} to.
         ...
--------------------------------------------------------------------------------*/
void *Material_load(char *path)
{
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

void material_set_texture_path(Material *material, char *texture_name, char *texture_resource_path)
{
    // Convenience function, because a resource handle need not be backed by a path.
    material_set_texture(material, texture_name, new_resource_handle(Texture, texture_resource_path));
}
void material_set_texture(Material *material, char *texture_name, ResourceHandle texture_resource_handle)
{
    MaterialType *mt = resource_data(MaterialType, material->material_type);

    for (int i = 0; i < mt->num_textures; i++) {
        if (strcmp(mt->texture_names[i], texture_name) == 0) {
            material->textures[i] = texture_resource_handle;
            return;
        }
    }
    fprintf(stderr, ERROR_ALERT "Attempted to set a material's texture named \"%s\", which is not a texture required for this material type.\n", texture_name);
    exit(EXIT_FAILURE);
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
void print_shader_blocks(void)
{
    printf("Shader blocks\n");
    printf("=============\n");
    for (int i = 0; i < g_num_shader_blocks; i++) {
        ___print_shader_block(i);
    }
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

    // Create a bool array of dirty flags for each entry in the flattened shader-block struct
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
