/*================================================================================
    Rendering and rendering resources module.
================================================================================*/
#ifndef HEADER_DEFINED_RENDERING
#define HEADER_DEFINED_RENDERING
#include <stdint.h>
/*--------------------------------------------------------------------------------
Graphics API definitions. This module is still dependent on OpenGL, but
things such as type names and enum mappings are typedef'd/macro'd.
     GraphicsID
     GraphicsInt
     GraphicsFloat
     GraphicsMat4x4f
     GraphicsUniformID
--------------------------------------------------------------------------------*/
#include "rendering/gl.h"
/*--------------------------------------------------------------------------------
This rendering module is built using the resources system. This is a shared
object system which gives "resources" identifiers as paths. Instead of holding
an object, a resource handle is held to make sure that objects with the same
identifiers are shared.
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
#define MAX_ATTRIBUTE_NAME_LENGTH 32
typedef struct AttributeInfo_s {
    AttributeType attribute_type;
    char name[MAX_ATTRIBUTE_NAME_LENGTH + 1];
    GLenum gl_type;
    GLuint gl_size;
} AttributeInfo;

#define ATTRIBUTE_BITMASK_SIZE 32
typedef uint32_t VertexFormat;
#define NUM_VERTEX_FORMATS 3
#define VERTEX_FORMAT_NONE 0
#define VERTEX_FORMAT_3 1 << ATTRIBUTE_TYPE_POSITION
#define VERTEX_FORMAT_C 1 << ATTRIBUTE_TYPE_COLOR
#define VERTEX_FORMAT_N 1 << ATTRIBUTE_TYPE_NORMAL
#define VERTEX_FORMAT_U 1 << ATTRIBUTE_TYPE_UV
#define VERTEX_FORMAT_3C VERTEX_FORMAT_3 | VERTEX_FORMAT_C
#define VERTEX_FORMAT_3N VERTEX_FORMAT_3 | VERTEX_FORMAT_N
#define VERTEX_FORMAT_3CN VERTEX_FORMAT_3 | VERTEX_FORMAT_C | VERTEX_FORMAT_N
#define VERTEX_FORMAT_3CU VERTEX_FORMAT_3 | VERTEX_FORMAT_C | VERTEX_FORMAT_U
// Translate from strings of the form "3CU", etc., to vertex format bitmasks.
// This is how they will be used in text files.
VertexFormat string_to_VertexFormat(char *string);
/*--------------------------------------------------------------------------------
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
#define GRAPHICS_PROGRAM_VF (1 << Vertex) & (1 << Fragment)
#define GRAPHICS_PROGRAM_VGF (1 << Vertex) & (1 << Geometry) & (1 << Fragment)
#define GRAPHICS_PROGRAM_VTTF (1 << Vertex) & (1 << TesselationControl) & (1 << TesselationEvaluation) & (1 << Fragment)
#define GRAPHICS_PROGRAM_VGTTF (1 << Vertex) & (1 << Geometry) & (1 << TesselationControl) & (1 << TesselationEvaluation) & (1 << Fragment)
/*--------------------------------------------------------------------------------
--------------------------------------------------------------------------------*/
extern ResourceType Shader_RTID;
typedef struct /* Resource */ Shader_s {
    ShaderType shader_type;
    GraphicsID shader_id;
} Shader;

/*--------------------------------------------------------------------------------
--------------------------------------------------------------------------------*/
typedef uint8_t UniformType;
enum UniformTypes {
    UNIFORM_GLOBAL_FLOAT,
    UNIFORM_GLOBAL_INT,
    UNIFORM_GLOBAL_MAT4X4F,
    UNIFORM_FLOAT,
    UNIFORM_INT,
    UNIFORM_MAT4X4F
};
#define MAX_UNIFORM_NAME_LENGTH 31
typedef union UniformGetter_u {
    GraphicsInt *global_int;
    GraphicsInt *global_float;
    GraphicsInt *global_mat4x4f;
    GraphicsInt (*int_getter)(void);
    GraphicsFloat (*float_getter)(void);
    GraphicsMat4x4f (*mat4x4f_getter)(void);
} UniformGetter;
typedef struct Uniform_s {
    char name[MAX_UNIFORM_NAME_LENGTH + 1];
    GraphicsUniformID location;
    UniformType type;
    UniformGetter getter;
} Uniform;
/*--------------------------------------------------------------------------------
--------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------
A "graphics program" is a linked set of shader objects. It holds these as resource
handles so that shader objects are not recompiled for each program they link into.
A graphics program is itself a resource.
--------------------------------------------------------------------------------*/
extern ResourceType GraphicsProgram_RTID;
typedef struct /* Resource */ GraphicsProgram_s {
    GraphicsID program_id;
    GraphicsProgramType program_type;
    ResourceHandle shaders[NUM_SHADER_TYPES]; /* Resources: Shader[] */
    VertexFormat vertex_format;
} GraphicsProgram;
void *GraphicsProgram_load(char *path);

/*--------------------------------------------------------------------------------
  Mesh stuff
--------------------------------------------------------------------------------*/
extern const AttributeInfo g_attribute_info[];

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
An "Artist" is what is used to draw objects. This structure associates a graphics program
with the uniform data and retrieval needed to provide the context enough to render with this
graphics program.
--------------------------------------------------------------------------------*/
extern ResourceType Artist_RTID; // Comment on resources+rendering system:
                                 //   An Artist is a "virtual" resource. It is not built
                                 //   from a collection of assets, but is defined in code
                                 //   when creating a new Artist, and its path just needs
                                 //   to be unique, such as "Virtual/artists/1".
typedef struct /* Resource */ Artist_s {
    ResourceHandle graphics_program; // Resource: GraphicsProgram
    uint16_t num_uniforms;
    Uniform *uniform_array;
} Artist;
void *Artist_load(char *path);
void Artist_add_uniform(Artist *artist, char *name, UniformGetter getter, UniformType uniform_type);
void Artist_bind(Artist *artist);
void Artist_draw_mesh(Artist *artist, Mesh *mesh);


/*--------------------------------------------------------------------------------
Shader bookkeeping stuff. Reading the source into application memory
(OpenGL requires this since it needs to send all the source to the driver to be compiled),
loading and compilation, and linking, with error handling and log printing.
--------------------------------------------------------------------------------*/
void read_shader_source(const char *name, char **lines_out[], size_t *num_lines);
bool load_and_compile_shader(GraphicsID shader_id, const char *shader_path);
void link_shader_program(GraphicsID shader_program_id);

/*--------------------------------------------------------------------------------
Loading stuff. Possibly should be outside of this module, but since a mesh loading
function explicitly accounts for possible ways to compile a mesh asset, these
are here.
--------------------------------------------------------------------------------*/
void load_mesh_ply(MeshData *mesh, VertexFormat vertex_format, FILE *file);

#endif // HEADER_DEFINED_RENDERING
