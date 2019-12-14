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
#define GRAPHICS_PROGRAM_VF (1 << Vertex) | (1 << Fragment)
#define GRAPHICS_PROGRAM_VGF (1 << Vertex) | (1 << Geometry) | (1 << Fragment)
#define GRAPHICS_PROGRAM_VTTF (1 << Vertex) | (1 << TesselationControl) | (1 << TesselationEvaluation) | (1 << Fragment)
#define GRAPHICS_PROGRAM_VGTTF (1 << Vertex) | (1 << Geometry) | (1 << TesselationControl) | (1 << TesselationEvaluation) | (1 << Fragment)
/*--------------------------------------------------------------------------------
--------------------------------------------------------------------------------*/
extern ResourceType Shader_RTID;
typedef struct /* Resource */ Shader_s {
    ShaderType shader_type;
    GraphicsID shader_id;
} Shader;
void *Shader_load(char *path);
bool Shader_reload(ResourceHandle handle);

/*--------------------------------------------------------------------------------
--------------------------------------------------------------------------------*/
typedef uint8_t UniformType;
enum UniformTypes {
    UNIFORM_NONE,
    UNIFORM_FLOAT,
    UNIFORM_INT,
    UNIFORM_MAT4X4F,
};
// The string for a uniform type is whatever GLSL uses for the variable type.
// This is so uniform types can be stored in text files, and the names are
// the same as would appear in uniform declarations in a GLSL source file.
UniformType string_to_UniformType(char *string);
#define MAX_UNIFORM_NAME_LENGTH 31
typedef union UniformGetter_u {
    GraphicsInt (*int_getter)(void);
    GraphicsFloat (*float_getter)(void);
    GraphicsMat4x4f (*mat4x4f_getter)(void);
} UniformGetter;
typedef struct Uniform_s {
    char name[MAX_UNIFORM_NAME_LENGTH + 1];
    UniformType type;
    GraphicsUniformID location;
    UniformGetter getter; //------remove this
} Uniform;
/*--------------------------------------------------------------------------------
--------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------
A "graphics program" is a linked set of shader objects. It holds these as resource
handles so that shader objects are not recompiled for each program they link into.
A graphics program is itself available as a resource.

Another thing encapsulated by a graphics program is the uniform interface. This
is a set of names and types for the uniform values required by this program. This
will depend on what shaders are linked and what uniforms they require. If standard
names are given for common uniform variables, organizing this may be easier.
This uniform-interface provides a way to "bind" a graphics program to an application,
using an "artist", which holds the graphics program as a resource and gives
uniform-retrieval information for each uniform required by the graphics program.
--------------------------------------------------------------------------------*/
#define UNIFORM_BLOCK_STANDARD 0
/* typedef uint8_t GraphicsProgramType; */
/* enum GraphicsProgramTypes { */
/*     GRAPHICS_PROGRAM_NONE, */
/*     GRAPHICS_PROGRAM_STANDARD_VIEW, */
/*     GRAPHICS_PROGRAM_STANDARD_2D, */
/* }; */
extern ResourceType GraphicsProgram_RTID;
typedef struct /* Resource */ GraphicsProgram_s {
    GraphicsID program_id;
    GraphicsProgramType program_type;
    ResourceHandle shaders[NUM_SHADER_TYPES]; /* Resources: Shader[] */
    VertexFormat vertex_format;
    uint16_t num_uniforms;
    Uniform *uniform_array;
} GraphicsProgram;
void *GraphicsProgram_load(char *path); // Load this from a .GraphicsProgram configuration file.
bool GraphicsProgram_reload(ResourceHandle handle);
void GraphicsProgram_add_uniform(GraphicsProgram *program, UniformType uniform_type, char *name);

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
An "Artist" is what is used to draw objects. This structure associates a graphics program
with the uniform-retrieval information needed to provide the context enough to render with this
graphics program. So, it is a sort of binding of the graphics program to the
application using the artist. While the graphics program has a consistent uniform-interface,
it does not know where to get these values when needed.

So, the "artist" is what collects data needed for drawing under a certain "style".
The other important parameter to a drawing command is a model, mesh, or other kind
of renderable object.
--------------------------------------------------------------------------------*/
extern ResourceType Artist_RTID; // Comment on resources+rendering system:
                                 //   An Artist is a "virtual" resource. It is not built
                                 //   from a collection of assets, but is defined in code
                                 //   when creating a new Artist, and its path just needs
                                 //   to be unique, such as "Virtual/artists/1".
typedef struct /* Resource */ Artist_s {
    ResourceHandle graphics_program; // Resource: GraphicsProgram
    void (*prepare) (struct Artist_s *);
    uint32_t uniform_flags[12]; //---Bitmask for active uniforms, n*32 allowed uniforms.
    uint16_t num_uniforms;
    Uniform *uniform_array;
} Artist;
void *Artist_load(char *path); // Virtual "load". Allocates an Artist object.

/* void Artist_prepare_float(Artist *artist, char *name, GraphicsFloat val); */
/* void Artist_prepare_mat4x4(Artist *artist, char *name, GraphicsMat4x4f val); */
/* void Artist_prepare_int(Artist *artist, char *name, GraphicsInt val); */

/* void Artist_prepare_float(Artist *artist, char *name, GraphicsFloat val) */
/* { */
/*     GraphicsProgram *program = resource_data(GraphicsProgram, artist->graphics_program); */
/*     artist->uniform_flags */
/* } */



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


#endif // HEADER_DEFINED_RENDERING
