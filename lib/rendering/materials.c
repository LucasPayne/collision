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
void MaterialType_load(void *resource, char *path)
{
    MaterialType mt = {0};
#define load_error(STRING)\
    { fprintf(stderr, "Error while loading MaterialType: " STRING "\n"); return NULL; }
    DD *dd = dd_open(g_resource_dictionary, path);
    if (dd == NULL) load_error("Could not open dictionary.\n");
    char *vertex_format_string;
    if (!dd_get(dd, "vertex_format", "string", &vertex_format_string)) load_error("No vertex_format.");
    mt.vertex_format = string_to_VertexFormat(vertex_format_string);
    if (mt.vertex_format == VERTEX_FORMAT_NONE) load_error("Bad vertex format.");
    if (!dd_get(dd, "num_blocks", "uint", &mt.num_blocks)) load_error("No num_blocks.");
    if (mt.num_blocks > MATERIAL_MAX_SHADER_BLOCKS) load_error("Too many shader blocks declared in num_blocks.");
    if (!dd_get(dd, "num_textures", "uint", &mt.num_textures)) load_error("No num_blocks.");
    if (mt.num_textures > MATERIAL_MAX_TEXTURES) load_error("Too many textures declared in num_textures.");

    // Attempt to find/load the Shader resources, and then link them.
    char *vertex_shader;
    if (!dd_get(dd, "vertex_shader", "string", &vertex_shader)) load_error("No vertex_shader.");
    char *fragment_shader;
    if (!dd_get(dd, "fragment_shader", "string", &fragment_shader)) load_error("No fragment_shader.");
    mt.program_type = GRAPHICS_PROGRAM_VF; // Only handling vertex+fragment programs currently.
    mt.shaders[Vertex] = new_resource_handle(Shader, vertex_shader);
    mt.shaders[Fragment] = new_resource_handle(Shader, fragment_shader);

    mt.program_id = glCreateProgram();
    // Attach the shaders. If they aren't loaded resources, then this loads and tries to compile them.
    glAttachShader(mt.program_id, resource_data(Shader, mt.shaders[Vertex])->shader_id);
    glAttachShader(mt.program_id, resource_data(Shader, mt.shaders[Fragment])->shader_id);
    // Link the program, and do error-checking.
    link_shader_program(mt.program_id);
    // Detach the shaders so they can be deleted if needed.
    glDetachShader(mt.program_id, resource_data(Shader, mt.shaders[Vertex])->shader_id);
    glDetachShader(mt.program_id, resource_data(Shader, mt.shaders[Fragment])->shader_id);

    // A file-backed material instance of this material-type defines its textures from their names given in this material type text-file. Collate these
    // names, and attach texture{i}'s declared texture name to texture unit GL_TEXTURE{i}.
    glUseProgram(mt.program_id); // use the program so sampler uniforms can be uploaded.
    for (int i = 0; i < mt.num_textures; i++) {
        char texture_token[32];
        sprintf(texture_token, "texture%d", i);
        char *texture;
        if (!dd_get(dd, texture_token, "string", &texture)) load_error("Not all declared textures have been given.");
        if (strlen(texture) >= MATERIAL_MAX_TEXTURE_NAME_LENGTH) load_error("Texture name too long.");
        strncpy(mt.texture_names[i], texture, MATERIAL_MAX_TEXTURE_NAME_LENGTH);
        // Bind this texture in the program to the binding point.
        GLint texture_location = glGetUniformLocation(mt.program_id, texture);
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
    for (int i = 0; i < mt.num_blocks; i++) {
        char block_token[32];
        sprintf(block_token, "block%d", i);
        char *block;
        if (!dd_get(dd, block_token, "string", &block)) load_error("Not all declared shader blocks have been given.");
        ShaderBlockID block_id = get_shader_block_id(block);
        if (block_id < 0) load_error("Unsupported shader block.");
        mt.shader_blocks[i] = block_id;
        // Bind the block to the linked program.
        GLuint block_index = glGetUniformBlockIndex(mt.program_id, block);
        if (block_index == GL_INVALID_INDEX) {
            // Hopefully the glsl compiler does not optimize away entire std140 uniform blocks.
            // (It appears that it doesn't.)
            load_error("Could not find uniform block.");
        }
        glUniformBlockBinding(mt.program_id, block_index, block_id);
        // The block id is both index into the global block info array, and the binding point.
        glBindBufferBase(GL_UNIFORM_BUFFER, block_id, g_shader_blocks[block_id].vram_buffer_id);
    }

    // There is a special shader block, MaterialProperties, which is the per-material-instance interface to the parameters
    // of this material type.
    // Collect material-properties information and store this in the info struct array of the material type.
    bool has_property_block = false;
    for (int i = 0; i < mt.num_blocks; i++) {
        if (mt.shader_blocks[i] == ShaderBlockID_MaterialProperties) {
            has_property_block = true;
            break;
        }
    }
    if (has_property_block) {
        GLuint mp_block_index = glGetUniformBlockIndex(mt.program_id, "MaterialProperties");
        if (mp_block_index == GL_INVALID_INDEX) {
            fprintf(stderr, ERROR_ALERT "Something went wrong. MaterialProperties block index cannot be found for material type which apparently uses one.\n");
            exit(EXIT_FAILURE);
        }
        const int max_num_properties = 512;
        glGetActiveUniformBlockiv(mt.program_id, mp_block_index, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &mt.num_properties);
        if (mt.num_properties > max_num_properties) {
            fprintf(stderr, ERROR_ALERT "Too many entries defined in a MaterialProperties block. The maximum is set to %d, but this can be changed.\n", max_num_properties);
            exit(EXIT_FAILURE);
        }
        GLuint uniform_indices[max_num_properties];
        glGetActiveUniformBlockiv(mt.program_id, mp_block_index, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, uniform_indices);

        glGetActiveUniformBlockiv(mt.program_id, mp_block_index, GL_UNIFORM_BLOCK_DATA_SIZE, &mt.properties_size);
        if (mt.properties_size > g_shader_blocks[ShaderBlockID_MaterialProperties].size) {
            fprintf(stderr, ERROR_ALERT "MaterialProperties block encountered which is too large. The maximum block size is %zu, which can be changed.\n", g_shader_blocks[ShaderBlockID_MaterialProperties].size);
            exit(EXIT_FAILURE);
        }

        // Collect information about the entries of the MaterialProperties uniform block.
        for (int i = 0; i < mt.num_properties; i++) {
            MaterialPropertyInfo info = {0};
            info.location = uniform_indices[i];
            int name_length;
            glGetActiveUniformName(mt.program_id, uniform_indices[i], MATERIAL_MAX_PROPERTY_NAME_LENGTH + 1, &name_length, info.name); //+1?
            if (name_length > MATERIAL_MAX_PROPERTY_NAME_LENGTH) {
                fprintf(stderr, ERROR_ALERT "Entry name \"%s\" in MaterialProperties uniform block is too long. The maximum name length is %d.\n", info.name, MATERIAL_MAX_PROPERTY_NAME_LENGTH);
                exit(EXIT_FAILURE);
            }
            info.name[name_length] = '\0';
            glGetActiveUniformsiv(mt.program_id, 1, &uniform_indices[i], GL_UNIFORM_TYPE, (GLint *) &info.type);
            glGetActiveUniformsiv(mt.program_id, 1, &uniform_indices[i], GL_UNIFORM_OFFSET, &info.offset);
            mt.property_infos[i] = info;
        }
    } else {
        mt.properties_size = 0;
        mt.num_properties = 0;
    }
    /* printf("Finished loading material type from path %s\n", path); */
    /* print_material_type(&material_type); */
    
    // Successfully filled the MaterialType.
    MaterialType *out_material_type = (MaterialType *) resource;
    memcpy(out_material_type, &mt, sizeof(MaterialType));
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
  Material instances
--------------------------------------------------------------------------------*/
void Material_load(void *resource, char *path)
{
    Material material = {0};
#define load_error(STRING)\
    { fprintf(stderr, "Error while loading Material: " STRING "\n"); return NULL; }
    DD *dd = dd_open(g_resource_dictionary, path);
    char *material_type_path;
    if (!dd_get(dd, "type", "string", &material_type_path)) load_error("No type.");
    material.material_type = new_resource_handle(MaterialType, material_type_path);
    MaterialType *mt = resource_data(MaterialType, material.material_type);

    // Create the texture resource handles for each texture of this material type.
    for (int i = 0; i < mt->num_textures; i++) {
        const int n = 2048;
        char texture_path[n];
        snprintf(texture_path, n, "%s/%s", path, mt->texture_names[i]); // Create the path to this subdictionary.
        material.textures[i] = new_resource_handle(Texture, texture_path);
    }
    
    // If there is a MaterialProperties block in this material's type, find its properties in the dd.
    if (mt->num_properties > 0 && mt->properties_size > 0) {
        // Allocate memory for the properties.
        material.properties = calloc(1, mt->properties_size);
        mem_check(material.properties);
        DD *properties = dd_open(dd, "properties");
        if (properties == NULL) load_error("No properties.");
        for (int i = 0; i < mt->num_properties; i++) {
            MaterialPropertyInfo *info = &mt->property_infos[i];
            // Convert the gl type into a type deserializable from dd files.
            char *type;
            #define set(gl_type,str) case gl_type: type = ( str ); break
            switch(info->type) {
                set(GL_FLOAT, "float");
                set(GL_BOOL, "bool");
                set(GL_FLOAT_VEC4, "vec4");
                set(GL_INT, "int");
                set(GL_UNSIGNED_INT, "uint");
                default:
                    load_error("Unsupported material property type.");
            }
            #undef set
            if (!dd_get(properties, info->name, type, material.properties + info->offset)) load_error("Failed to read material proeprty.");
        }
    } else {
        // If there is no material properties block, the size is zero. So, do not allocate memory for properties.
        material.properties = NULL;
    }
    printf("Finished loading file-backed material instance from path %s\n", path);

    Material *out_material = (Material *) resource;
    memcpy(out_material, &material, sizeof(Material));
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
