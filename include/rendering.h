/*================================================================================
================================================================================*/
#ifndef HEADER_DEFINED_RENDERING
#define HEADER_DEFINED_RENDERING
#include <stdint.h>
/*--------------------------------------------------------------------------------
Graphics API definitions. This module is still dependent on the OpenGL, but
things such as type names are typedef'd.
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
#define ATTRIBUTE_BITMASK_SIZE 32
typedef uint32_t VertexFormat;
#define NUM_VERTEX_FORMATS 3
#define VERTEX_FORMAT_3 1 << ATTRIBUTE_TYPE_POSITION
#define VERTEX_FORMAT_C 1 << ATTRIBUTE_TYPE_COLOR
#define VERTEX_FORMAT_N 1 << ATTRIBUTE_TYPE_NORMAL
#define VERTEX_FORMAT_U 1 << ATTRIBUTE_TYPE_UV
#define VERTEX_FORMAT_3C VERTEX_FORMAT_3 | VERTEX_FORMAT_C
#define VERTEX_FORMAT_3N VERTEX_FORMAT_3 | VERTEX_FORMAT_N
#define VERTEX_FORMAT_3CN VERTEX_FORMAT_3 | VERTEX_FORMAT_C | VERTEX_FORMAT_N
#define VERTEX_FORMAT_3CU VERTEX_FORMAT_3 | VERTEX_FORMAT_C | VERTEX_FORMAT_U
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
typedef struct Uniform_s {
    char name[MAX_UNIFORM_NAME_LENGTH + 1];
    GraphicsUniformID location;
    UniformType type;
    union UniformGetter {
        GraphicsInt *global_int_getter;
        GraphicsInt *global_float_getter;
        GraphicsInt *global_mat4x4f_getter;
        GraphicsInt (*int_getter)(void);
        GraphicsFloat (*float_getter)(void);
        GraphicsMat4x4f (*mat4x4f_getter)(void);
    } getter;
} Uniform;
/*--------------------------------------------------------------------------------
--------------------------------------------------------------------------------*/
extern ResourceType GraphicsProgram_RTID;
typedef struct /* Resource */ GraphicsProgram_s {
    GraphicsID program_id;
    GraphicsProgramType program_type;
    ResourceHandle shaders[NUM_SHADER_TYPES]; /* Resources: Shader[] */
    VertexFormat vertex_format;
    uint16_t num_uniforms;
    Uniform *uniform_array;
} GraphicsProgram;

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


/*--------------------------------------------------------------------------------
Shader bookkeeping stuff. Reading the source into application memory
(OpenGL requires this since it needs to send all the source to the driver to be compiled),
loading and compilation, and linking, with error handling and log printing.
--------------------------------------------------------------------------------*/
void read_shader_source(const char *name, char **lines_out[], size_t *num_lines);
bool load_and_compile_shader(GraphicsID shader_id, const char *shader_path);
void link_shader_program(GraphicsID shader_program_id);

#endif // HEADER_DEFINED_RENDERING
