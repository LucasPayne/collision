/*================================================================================
    Rendering and rendering resources module.
================================================================================*/
#ifndef HEADER_DEFINED_RENDERING
#define HEADER_DEFINED_RENDERING
#include <stdint.h>
/*--------------------------------------------------------------------------------
Graphics API definitions. This module is still dependent on OpenGL, but
things such as type names and enum mappings are typedef'd/macro'd. With
correct typedefs it is less easy to make the mistake of, for example,
retrieving a uniform location as a GLuint. -1 is used for a failed
retrieval, and this will not be caught.
     GraphicsID
     GraphicsInt
     GraphicsFloat
     GraphicsMat4x4f
     GraphicsUniformID
--------------------------------------------------------------------------------*/
#include "rendering/gl.h"
/*--------------------------------------------------------------------------------
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
    NUM_ATTRIBUTE_TYPES
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
#define VERTEX_FORMAT_3C (VERTEX_FORMAT_3 | VERTEX_FORMAT_C)
#define VERTEX_FORMAT_3N (VERTEX_FORMAT_3 | VERTEX_FORMAT_N)
#define VERTEX_FORMAT_3CN (VERTEX_FORMAT_3 | VERTEX_FORMAT_C | VERTEX_FORMAT_N)
#define VERTEX_FORMAT_3CU (VERTEX_FORMAT_3 | VERTEX_FORMAT_C | VERTEX_FORMAT_U)
// Translate from strings of the form "3CU", etc., to vertex format bitmasks.
// This is how they will be used in text files.
VertexFormat string_to_VertexFormat(char *string);
/*--------------------------------------------------------------------------------
    Shader types and graphics program types
--------------------------------------------------------------------------------*/
typedef uint8_t ShaderType;
enum ShaderTypes {
    Vertex,
    Fragment,
    Geometry,
    TesselationControl,
    TesselationEvaluation,
    NUM_SHADER_TYPES
};
typedef uint8_t GraphicsProgramType;
#define GRAPHICS_PROGRAM_VF ((1 << Vertex) | (1 << Fragment))
#define GRAPHICS_PROGRAM_VGF ((1 << Vertex) | (1 << Geometry) | (1 << Fragment))
#define GRAPHICS_PROGRAM_VTTF ((1 << Vertex) | (1 << TesselationControl) | (1 << TesselationEvaluation) | (1 << Fragment))
#define GRAPHICS_PROGRAM_VGTTF ((1 << Vertex) | (1 << Geometry) | (1 << TesselationControl) | (1 << TesselationEvaluation) | (1 << Fragment))
/*--------------------------------------------------------------------------------
    Shader resources
--------------------------------------------------------------------------------*/
extern ResourceType Shader_RTID;
typedef struct /* Resource */ Shader_s {
    ShaderType shader_type;
    GraphicsID shader_id;
} Shader;
void *Shader_load(char *path);
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
} ShaderBlockInfo;
#define print_shader_block(BLOCK_NAME)\
    ___print_shader_block(( ShaderBlockID_ ## BLOCK_NAME ))
void ___print_shader_block(ShaderBlockID id);
#define MAX_NUM_SHADER_BLOCKS 128
extern ShaderBlockInfo g_shader_blocks[MAX_NUM_SHADER_BLOCKS];

#define add_shader_block(BLOCK_NAME)\
    ___add_shader_block(&( ShaderBlockID_ ## BLOCK_NAME ),\
                        ( sizeof(ShaderBlock_ ## BLOCK_NAME) ),\
                        ( #BLOCK_NAME ))
void ___add_shader_block(ShaderBlockID *id_pointer, size_t size, char *name);

#define set_uniform_float(BLOCK_NAME,ENTRY,FLOAT_VALUE)\
    ___set_uniform_float(( ShaderBlockID_ ## BLOCK_NAME ),\
                         &(( ((ShaderBlock_ ## BLOCK_NAME *) g_shader_blocks[( ShaderBlockID_ ## BLOCK_NAME )].shader_block)->ENTRY)),\
                         ( FLOAT_VALUE ))
/* Pointer arithmetic can be used if needed, to calculate the offset (it is used for bitflags.) */

ShaderBlockID get_shader_block_id(char *name);
void ___set_uniform_float(ShaderBlockID id, float *entry_address, float val);

/*--------------------------------------------------------------------------------
  Material types
--------------------------------------------------------------------------------*/
#define MATERIAL_MAX_SHADER_BLOCKS 64
#define MATERIAL_MAX_TEXTURES 80
#define MATERIAL_MAX_TEXTURE_NAME_LENGTH 64
extern ResourceType MaterialType_RTID;
typedef struct /* Resource */ MaterialType_s {
    VertexFormat vertex_format;
    ResourceHandle shaders[NUM_SHADER_TYPES];
    int num_blocks;
    /* ShaderBlockID shader_blocks[MATERIAL_MAX_SHADER_BLOCKS]; */
    uint16_t shader_blocks[MATERIAL_MAX_SHADER_BLOCKS];
    int num_textures;
    char texture_names[MATERIAL_MAX_TEXTURE_NAME_LENGTH + 1][MATERIAL_MAX_TEXTURES];

    GraphicsProgramType program_type;
    GLuint program_id;
} MaterialType;
void *MaterialType_load(char *path);

/*--------------------------------------------------------------------------------
    Material instances
--------------------------------------------------------------------------------*/
extern ResourceType Material_RTID;
typedef struct /* Resource */ Material_RTID {
    ResourceHandle material_type; /* Resource: MaterialType */
    ResourceHandle textures[MATERIAL_MAX_TEXTURES]; /* Resource[]: Texture */
} Material;
void *Material_load(char *path);

/*--------------------------------------------------------------------------------
  Mesh stuff
--------------------------------------------------------------------------------*/
typedef struct MeshData_s {
    VertexFormat vertex_format;
    uint32_t num_vertices;
    void *attribute_data[NUM_ATTRIBUTE_TYPES];
    uint32_t num_triangles;
    uint32_t *triangles;
} MeshData;

extern ResourceType Mesh_RTID;
typedef struct /* Resource */ Mesh_s {
    VertexFormat vertex_format;
    uint32_t num_vertices;
    GraphicsID vertex_array_id;
    GraphicsID attribute_buffer_ids[NUM_ATTRIBUTE_TYPES];
    uint32_t num_triangles;
    GraphicsID triangles_id;
} Mesh;
void *Mesh_load(char *path);
void upload_mesh(Mesh *mesh, MeshData *mesh_data);

/*--------------------------------------------------------------------------------
Shader bookkeeping stuff. Reading the source into application memory
(OpenGL requires this since it needs to send all the source to the driver to be compiled),
loading and compilation, and linking, with error handling and log printing.
--------------------------------------------------------------------------------*/
void read_shader_source(const char *name, char **lines_out[], size_t *num_lines);
bool load_and_compile_shader(GraphicsID shader_id, const char *shader_path);
bool link_shader_program(GraphicsID shader_program_id);

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
void *Texture_load(char *path);


/*--------------------------------------------------------------------------------
  Text file configuration, dictionaries integration
--------------------------------------------------------------------------------*/
#include "dictionary.h"
void dict_query_rules_rendering(DictQuerier *q);

#endif // HEADER_DEFINED_RENDERING
