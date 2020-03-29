/*================================================================================
    Rendering and rendering resources module.
================================================================================*/
#ifndef HEADER_DEFINED_RENDERING
#define HEADER_DEFINED_RENDERING
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdint.h>

// glsl utilities such as shader compilation and linking are separated, so they can be tested without the rendering module's dependencies.
#include "glsl_utilities.h"

// note: It may be a convenient dependency to have the matrix types in this module. If arithmetic isn't done with them,
// then this will just require a header include.
#include "matrix_mathematics.h"

/*--------------------------------------------------------------------------------
    Resources
This rendering module is built using the resources system. This is a shared
object system which gives "resources" identifiers as "paths". Instead of holding
an object, a resource handle is held to make sure that objects with the same
identifiers are shared.

Notes on the resource system and its use here:
    "Paths" in the resource system may or may not be associated with assets. Asset-associated
    resources (such as a shader or a graphics program or a mesh) will build or load
    this resource from files related to that path by the global path-variable, attaching
    "drives" to physical paths. "Virtual" resources do not use this path to look up
    asset data, so could be prepended by "Virtual/", if this is not a physical drive.
    An example of this use is in an "Artist" resource.
--------------------------------------------------------------------------------*/
#include "resources.h"
void init_resources_rendering(void);
/*--------------------------------------------------------------------------------
    Vertex attributes and vertex formats.
Vertex formats are accounted for with bitmasks. The bitmask may be lengthened
later if needed. This allows quick superset/subset testing for vertex format
compatability.
--------------------------------------------------------------------------------*/
typedef uint8_t AttributeType;
enum AttributeTypes {
    ATTRIBUTE_TYPE_POSITION,
    ATTRIBUTE_TYPE_COLOR,
    ATTRIBUTE_TYPE_NORMAL,
    ATTRIBUTE_TYPE_UV,
    ATTRIBUTE_TYPE_TANGENT,
    NUM_ATTRIBUTE_TYPES
};
enum AttributeTypes2 { // better names, don't use this capitalization otherwise.
    Position,
    Color,
    Normal,
    TexCoord,
    Tangent,
};
// Attribute types are associated to an AttributeInfo structure by their value as an index.
// This attribute information is stored in a global array.
#define MAX_ATTRIBUTE_NAME_LENGTH 32
typedef struct AttributeInfo_s {
    AttributeType attribute_type;
    char name[MAX_ATTRIBUTE_NAME_LENGTH + 1];
    GLenum gl_type;
    GLuint gl_size;
} AttributeInfo;
extern const AttributeInfo g_attribute_info[];

#define ATTRIBUTE_BITMASK_SIZE 32
typedef uint32_t VertexFormat;
#define NUM_VERTEX_FORMATS 3
#define VERTEX_FORMAT_NONE 0
#define VERTEX_FORMAT_3 (1 << ATTRIBUTE_TYPE_POSITION)
#define VERTEX_FORMAT_C (1 << ATTRIBUTE_TYPE_COLOR)
#define VERTEX_FORMAT_N (1 << ATTRIBUTE_TYPE_NORMAL)
#define VERTEX_FORMAT_U (1 << ATTRIBUTE_TYPE_UV)
#define VERTEX_FORMAT_T (1 << ATTRIBUTE_TYPE_TANGENT)
#define VERTEX_FORMAT_3C (VERTEX_FORMAT_3 | VERTEX_FORMAT_C)
#define VERTEX_FORMAT_3N (VERTEX_FORMAT_3 | VERTEX_FORMAT_N)
#define VERTEX_FORMAT_3U (VERTEX_FORMAT_3 | VERTEX_FORMAT_U)
#define VERTEX_FORMAT_3NU (VERTEX_FORMAT_3 | VERTEX_FORMAT_N | VERTEX_FORMAT_U)
#define VERTEX_FORMAT_3CN (VERTEX_FORMAT_3 | VERTEX_FORMAT_C | VERTEX_FORMAT_N)
#define VERTEX_FORMAT_3CU (VERTEX_FORMAT_3 | VERTEX_FORMAT_C | VERTEX_FORMAT_U)
#define VERTEX_FORMAT_3CNU (VERTEX_FORMAT_3 | VERTEX_FORMAT_C | VERTEX_FORMAT_N | VERTEX_FORMAT_U)
// Translate from strings of the form "3CU", etc., to vertex format bitmasks.
// This is how they will be used in text files.
VertexFormat string_to_VertexFormat(char *string);
void print_vertex_format(VertexFormat vertex_format); // prints the bits of the vertex format
/*--------------------------------------------------------------------------------
    Shader types and graphics program types
--------------------------------------------------------------------------------*/
typedef uint8_t ShaderType;
enum ShaderTypes {
    Vertex,
    Fragment,
    Geom,
    TesselationControl,
    TesselationEvaluation,
    NUM_SHADER_TYPES
};
typedef uint8_t GraphicsProgramType;
#define GRAPHICS_PROGRAM_VF ((1 << Vertex) | (1 << Fragment))
#define GRAPHICS_PROGRAM_VGF ((1 << Vertex) | (1 << Geom) | (1 << Fragment))
#define GRAPHICS_PROGRAM_VTF ((1 << Vertex) | (1 << TesselationEvaluation) | (1 << Fragment))
#define GRAPHICS_PROGRAM_VTTF ((1 << Vertex) | (1 << TesselationControl) | (1 << TesselationEvaluation) | (1 << Fragment))
#define GRAPHICS_PROGRAM_VGTTF ((1 << Vertex) | (1 << Geom) | (1 << TesselationControl) | (1 << TesselationEvaluation) | (1 << Fragment))

typedef uint8_t PrimitiveType;
enum PrimitiveTypes {
    Triangles,
    Lines,
    Patches,
};
/*--------------------------------------------------------------------------------
    Shader resources
--------------------------------------------------------------------------------*/
extern ResourceType Shader_RTID;
typedef struct /* Resource */ Shader_s {
    ShaderType shader_type;
    GLuint shader_id;
} Shader;
void Shader_load(void *resource, char *path);
bool Shader_reload(ResourceHandle handle);

/*--------------------------------------------------------------------------------
    Shader blocks (buffer-backed shared uniform blocks)
--------------------------------------------------------------------------------*/

typedef int16_t ShaderBlockID; // null: -1
typedef struct ShaderBlockInfo_s {
    bool dirty;
    bool *dirty_flags;
    char *name;
    void *shader_block;
    ShaderBlockID id;
    GLuint vram_buffer_id;
    size_t size;

    int num_samplers;
    int samplers_start_index;
    char **sampler_names;
    GLuint *samplers;
} ShaderBlockInfo;
#define print_shader_block(BLOCK_NAME)\
    ___print_shader_block(( ShaderBlockID_ ## BLOCK_NAME ))
void ___print_shader_block(ShaderBlockID id);
#define MAX_NUM_SHADER_BLOCKS 64
extern int g_num_shader_blocks;
extern int g_num_reserved_samplers;
extern ShaderBlockInfo g_shader_blocks[MAX_NUM_SHADER_BLOCKS];

void print_shader_blocks(void);

#define add_shader_block(BLOCK_NAME)\
    ___add_shader_block(&( ShaderBlockID_ ## BLOCK_NAME ),\
                        ( sizeof(ShaderBlock_ ## BLOCK_NAME) ),\
                        ( #BLOCK_NAME ),\
                        ( ShaderBlockNumSamplers_ ## BLOCK_NAME ),\
                        ( ShaderBlockSamplerNames_ ## BLOCK_NAME ))
void ___add_shader_block(ShaderBlockID *id_pointer, size_t size, char *name, int num_samplers, char **sampler_names);

#define set_uniform_float(BLOCK_NAME,ENTRY,FLOAT_VALUE)\
    ___set_uniform_float(( ShaderBlockID_ ## BLOCK_NAME ),\
                         &(( ((ShaderBlock_ ## BLOCK_NAME *) g_shader_blocks[( ShaderBlockID_ ## BLOCK_NAME )].shader_block)->ENTRY)),\
                         ( FLOAT_VALUE ))
#define set_uniform_mat4x4(BLOCK_NAME,ENTRY,MAT4X4_POINTER)\
    ___set_uniform_mat4x4(( ShaderBlockID_ ## BLOCK_NAME ),\
                         (( ((ShaderBlock_ ## BLOCK_NAME *) g_shader_blocks[( ShaderBlockID_ ## BLOCK_NAME )].shader_block)->ENTRY)),\
                         ( MAT4X4_POINTER ))
#define set_uniform_mat3x3(BLOCK_NAME,ENTRY,POINTER)\
    ___set_uniform_mat3x3(( ShaderBlockID_ ## BLOCK_NAME ),\
                         (( ((ShaderBlock_ ## BLOCK_NAME *) g_shader_blocks[( ShaderBlockID_ ## BLOCK_NAME )].shader_block)->ENTRY)),\
                         ( POINTER ))
#define set_uniform_bool(BLOCK_NAME,ENTRY,VALUE)\
    ___set_uniform_bool(( ShaderBlockID_ ## BLOCK_NAME ),\
                         &(( ((ShaderBlock_ ## BLOCK_NAME *) g_shader_blocks[( ShaderBlockID_ ## BLOCK_NAME )].shader_block)->ENTRY)),\
                         ( VALUE ))
#define set_uniform_int(BLOCK_NAME,ENTRY,VALUE)\
    ___set_uniform_int(( ShaderBlockID_ ## BLOCK_NAME ),\
                         &(( ((ShaderBlock_ ## BLOCK_NAME *) g_shader_blocks[( ShaderBlockID_ ## BLOCK_NAME )].shader_block)->ENTRY)),\
                         ( VALUE ))
#define set_uniform_vec3(BLOCK_NAME,ENTRY,VALUE)\
    ___set_uniform_vec3(( ShaderBlockID_ ## BLOCK_NAME ),\
                         &(( ((ShaderBlock_ ## BLOCK_NAME *) g_shader_blocks[( ShaderBlockID_ ## BLOCK_NAME )].shader_block)->ENTRY)),\
                         ( VALUE ))
#define set_uniform_vec4(BLOCK_NAME,ENTRY,VALUE)\
    ___set_uniform_vec4(( ShaderBlockID_ ## BLOCK_NAME ),\
                         &(( ((ShaderBlock_ ## BLOCK_NAME *) g_shader_blocks[( ShaderBlockID_ ## BLOCK_NAME )].shader_block)->ENTRY)),\
                         ( VALUE ))
// Sampler variant. In the background, different stuff is done in both the program and glsl, but it is the same syntax.
// The syntax is achieved with a macro by calculating the index of the sampler in this shaderblock by pointer arithmetic with the entries of a
// global (unused) struct. This is a compile-time constant, so the struct is there just so this syntax works with a macro.
//#define set_uniform_sampler(BLOCK_NAME,SAMPLER_ENTRY,SAMPLER_ID)\
//    ___set_uniform_sampler(( ShaderBlockID_ ## BLOCK_NAME ),\
//                           &( ShaderBlockSamplers_ ## BLOCK_NAME.SAMPLER_ENTRY ) - &( ShaderBlockSamplers_ ## BLOCK_NAME ),\
//                           ( SAMPLER_ID ))
#define set_uniform_texture(BLOCK_NAME,SAMPLER_ENTRY,TEXTURE_ID)\
    ___set_uniform_texture(( ShaderBlockID_ ## BLOCK_NAME ),\
                           &( ShaderBlockSamplers_ ## BLOCK_NAME.SAMPLER_ENTRY ) - (GLint *) &( ShaderBlockSamplers_ ## BLOCK_NAME ),\
                           ( TEXTURE_ID ))

/* Pointer arithmetic can be used if needed, to calculate the offset (it is used for bitflags.) */
ShaderBlockID get_shader_block_id(char *name);
void ___set_uniform_float(ShaderBlockID id, float *entry_address, float val);
void ___set_uniform_mat4x4(ShaderBlockID id, float *entry_address, float *vals);
void ___set_uniform_bool(ShaderBlockID id, bool *entry_address, bool val);
void ___set_uniform_vec3(ShaderBlockID id, vec3 *entry_address, vec3 val);
void ___set_uniform_vec4(ShaderBlockID id, vec4 *entry_address, vec4 val);
void ___set_uniform_sampler(ShaderBlockID id, int sampler_index, GLuint sampler_id);
void ___set_uniform_texture(ShaderBlockID id, int shaderblock_sampler_index, GLuint texture_id);
void synchronize_shader_blocks(void);

/*--------------------------------------------------------------------------------
  Material types
--------------------------------------------------------------------------------*/
////// note: This resource consists of a lot of string data. Possibly it would be best
// not to provide for the maximal size, but to somehow have this as a dynamic resource.
// Static components would be in the root struct, then it would point to a string table which
// it will free when it is unloaded.
#define MATERIAL_MAX_SHADER_BLOCKS 4
#define MATERIAL_MAX_TEXTURES 4
#define MATERIAL_MAX_TEXTURE_NAME_LENGTH 32
#define MATERIAL_MAX_PROPERTIES 16
#define MATERIAL_MAX_PROPERTY_NAME_LENGTH 32
typedef struct MaterialPropertyInfo_s {
    char name[MATERIAL_MAX_PROPERTY_NAME_LENGTH + 1];
    GLint location;
    GLsizei offset;
    GLenum type;
} MaterialPropertyInfo;

extern ResourceType MaterialType_RTID;
typedef struct /* Resource */ MaterialType_s {
    VertexFormat vertex_format;
    ResourceHandle shaders[NUM_SHADER_TYPES];
    int num_blocks;
    /* ShaderBlockID shader_blocks[MATERIAL_MAX_SHADER_BLOCKS]; */
    uint16_t shader_blocks[MATERIAL_MAX_SHADER_BLOCKS];

    int num_textures;
    // The texture unit to bind to is the index.
    char texture_names[MATERIAL_MAX_TEXTURES][MATERIAL_MAX_TEXTURE_NAME_LENGTH + 1];

    size_t properties_size;
    int num_properties;
    MaterialPropertyInfo property_infos[MATERIAL_MAX_PROPERTIES];

    GraphicsProgramType program_type;
    GLuint program_id;

    // A material-type can force geometry it is used to render to be interpreted as patch data.
    bool force_patches;
    int patch_vertices;
} MaterialType;
void MaterialType_load(void *resource, char *path);
void print_material_type(MaterialType *material_type);


/*--------------------------------------------------------------------------------
    Material instances
--------------------------------------------------------------------------------*/
extern ResourceType Material_RTID;
typedef struct /* Resource */ Material_RTID {
    // note: a material instance is very big because it holds all the possible textures. Since they just need to be walked from the start, maybe
    // holds a linked list or a dynamic array.
    ResourceHandle material_type; /* Resource: MaterialType */
    ResourceHandle textures[MATERIAL_MAX_TEXTURES]; /* Resource[]: Texture */
    void *properties;
} Material;
void Material_load(void *resource, char *path);
void Material_unload(void *resource);
// Create a oneoff material, not backed by a dictionary, by the path of a material type.
ResourceHandle Material_create(char *material_type_path);

// This is the (currently) only given-from-the-start shader block. This is not meant to be
// accessed with the macro syntax, but corresponds to a buffer of a given size
// (MATERIAL_PROPERTIES_MAX_SIZE), and is used whenever you prepare to draw with
// a material instance. A material instance holds its own data, whether it is skeletoned
// by a struct so the user can modify the data that way, or if its read from a file, etc.,
// it does not matter. The data attached to the material is copied into the lower bytes of
// the application-level buffer for this shader block, and then when synchronize_shader_blocks
// is called, this is uploaded. So each material using shaders with a MaterialProperties block may have it defined
// in different ways, possibly reflecting in the application by a padded struct, or having some sort of automated
// serialization stuff for editing material properties.
// This means there is one buffer only for holding material properties.
//  --- Remember that code still has to be run in the application base to initialize this shader block.
ShaderBlockID ShaderBlockID_MaterialProperties;
#define MATERIAL_PROPERTIES_MAX_SIZE 1024
typedef struct ShaderBlock_MaterialProperties_s {
    uint8_t props[MATERIAL_PROPERTIES_MAX_SIZE];
} ShaderBlock_MaterialProperties;
//----Since this is a shader block, everything being generated by the gen_shader_blocks utility is hardcoded here.
#define ShaderBlockNumSamplers_MaterialProperties 0
struct ___ShaderBlockSamplers_MaterialProperties {
} ShaderBlockSamplers_MaterialProperties;
static char *ShaderBlockSamplerNames_MaterialProperties[] = { };


/*--------------------------------------------------------------------------------
  Geometry stuff
--------------------------------------------------------------------------------*/
// MeshData is the application-level data for triangle meshes, intended for standard
// mesh+material pair rendering.
// --- Right now a PLY upload looks like this:
// ---      - Read into some queried format
// ---      - Format into a MeshData
// ---      - Pass into geometry specifying stuff
// ---      - That passes it into vram
// ---      This is too many copies ...
typedef struct MeshData_s {
    VertexFormat vertex_format;
    uint32_t num_vertices;
    void *attribute_data[NUM_ATTRIBUTE_TYPES];
    size_t attribute_data_sizes[NUM_ATTRIBUTE_TYPES];
    uint32_t num_triangles;
    uint32_t *triangles;
} MeshData;
// This function allows the creation or replacement of normals in a MeshData, calculated from the mesh.
void MeshData_calculate_normals(MeshData *mesh_data);
// MeshData can be loaded optionally with the generation of UV coordinates.
void MeshData_calculate_uv_orthographic(MeshData *mesh_data, vec3 direction, float scale);
// Tangents are always calculated, when in the vertex format, from normals and uv coordinates.
void MeshData_calculate_tangents(MeshData *mesh_data);

extern ResourceType Geometry_RTID;
typedef struct /* Resource */ Geometry_s {
    GLuint vao_id;
    bool dynamic;
    VertexFormat vertex_format;
    PrimitiveType primitive_type;
    bool is_indexed;
    GLuint buffer_ids[NUM_ATTRIBUTE_TYPES];
    GLuint indices_id;
    int num_indices;
    int num_vertices;
    float radius;
    int patch_vertices; // meaningful only if primitive_type is Patches.

    MeshData *mesh_data; // if this is null, it has been freed from application memory. This is the default.
} Geometry;
// Like all resources, this structure does not need to be used as a shared resource or loaded with this function.
// This is for instancing and file-backed mesh rendering.
// Really need to think more about "virtual resources" and if any of this makes sense.
void Geometry_load(void *resource, char *path);
Geometry upload_mesh(MeshData *mesh_data);

#define GM_ATTRIBUTE_BUFFER_SIZE (1024*1024)
#define GM_INDEX_BUFFER_SIZE (1024*1024)

uint32_t attribute_1u(AttributeType attribute_type, uint32_t u);
uint32_t attribute_2f(AttributeType attribute_type, float a, float b);
uint32_t attribute_3f(AttributeType attribute_type, float a, float b, float c);
void attribute_buf(AttributeType attribute_type, void *buf, int count);
void gm_index(uint32_t index);
void gm_index_buf(uint32_t *indices, int count);
void gm_triangles(VertexFormat vertex_format);
Geometry gm_done(void);
void gm_draw(Geometry geometry, Material *material);
void gm_lines(VertexFormat vertex_format);
void gm_free(Geometry geometry);

/*--------------------------------------------------------------------------------
Loading stuff. Possibly should be outside of this module, but since a mesh loading
function explicitly accounts for possible ways to compile a mesh asset, these
are here.
--------------------------------------------------------------------------------*/
void load_mesh_ply(MeshData *mesh, VertexFormat vertex_format, FILE *file); 

/*--------------------------------------------------------------------------------
  Images and texturing
--------------------------------------------------------------------------------*/
typedef struct ImageData_s {
    uint32_t width;
    uint32_t height;
    /* GLenum swizzle[4]; // Swizzle for RGBA */
    GLenum external_format; // Channels, is-integer e.g. GL_RG
    GLenum external_type;   // Data type e.g. GL_UNSIGNED_SHORT, so the format in all is two unsigned 16-bit channels packed into data.
    void *data;
} ImageData;

extern ResourceType Texture_RTID;
typedef struct /* Resource */ Texture_s {
    GLuint texture_id;
} Texture;
void Texture_load(void *resource, char *path);

//---May be better somewhere else.
bool load_image_png(ImageData *image_data, FILE *file);

/*--------------------------------------------------------------------------------
    Fonts (implemented with signed distance fields for vector rendering with low resolution glyph textures).
--- This may be better separated.
--------------------------------------------------------------------------------*/
typedef struct Glyph_s {
    char character;
    float uvs[4]; // uv coordinates on the sdf glyph map of the sdf cell. Top-left, then bottom-right.
    float glyph_uvs[4]; // uv coordinates of the subrectangle containing the actual glyph extents, with no added reach.
    int width;
    int height; // Width and height of the glyph's bounding box, in pixels.
    int bearing_x;
    int bearing_y;
    int advance; // in 1/64ths of pixels, as retrieved with freetype.
} Glyph;
void print_glyph(Glyph *glyph);

ResourceType Font_RTID;
typedef struct /* Resource */ Font_s {
    GLuint sdf_glyph_map;
//---should rather be an actual map.
    int num_glyphs;
    Glyph *glyphs;
} Font;
void Font_load(void *resource, char *path);

//--------------------------------------------------------------------------------
// Working on currently
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void material_set_property_float(Material *material, char *property_name, float val);
void material_set_property_vec4(Material *material, char *property_name, vec4 v);
void material_set_property_bool(Material *material, char *property_name, bool val);
void material_set_texture_path(Material *material, char *texture_name, char *texture_resource_path);
void material_set_texture(Material *material, char *texture_name, ResourceHandle texture_resource_handle);


//////////////////////////////////////////////////////////////////////////////////
// working on
void material_prepare(Material *material);

GLenum gl_shader_type(ShaderType shader_type);

#endif // HEADER_DEFINED_RENDERING
