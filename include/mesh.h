/*================================================================================
   Mesh and mesh rendering module.
================================================================================*/
#ifndef HEADER_DEFINED_MESH
#define HEADER_DEFINED_MESH
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdint.h>
#include "matrix_mathematics.h"
#include "helper_definitions.h"
#include "helper_gl.h"

/*================================================================================
Vertex formats
--------------
Vertex format information is kept in static application memory and is initialized
by a call to an initialization function in this module.
================================================================================*/


typedef uint8_t AttributeType;
// Corresponds to layout qualified positions in shaders
enum AttributeTypes {
    ATTRIBUTE_TYPE_POSITION,
    ATTRIBUTE_TYPE_COLOR,
    ATTRIBUTE_TYPE_NORMAL,
    NUM_ATTRIBUTE_TYPES
};

typedef struct AttributeInfo_s {
    AttributeType attribute_type;
    char name[MAX_ATTRIBUTE
    GLenum gl_type;
    GLuint gl_size;
} AttributeInfo;

typedef uint32_t VertexFormat;
static int NUM_VERTEX_FORMATS = 3;
static VertexFormat VERTEX_FORMAT_3 = 1 << ATTRIBUTE_TYPE_POSITION;
static VertexFormat VERTEX_FORMAT_C = 1 << ATTRIBUTE_TYPE_COLOR;
static VertexFormat VERTEX_FORMAT_N = 1 << ATTRIBUTE_TYPE_NORMAL;
static VertexFormat VERTEX_FORMAT_3C = VERTEX_FORMAT_3 | VERTEX_FORMAT_C;
static VertexFormat VERTEX_FORMAT_3N = VERTEX_FORMAT_3 | VERTEX_FORMAT_N;
static VertexFormat VERTEX_FORMAT_3CN = VERTEX_FORMAT_3 | VERTEX_FORMAT_C | VERTEX_FORMAT_N;


#define MAX_VERTEX_FORMAT_NAME_LENGTH 63
#define ATTRIBUTE_NAME_DATA_LENGTH 512
#define MAX_ATTRIBUTE_NAME_LENGTH 24
typedef struct VertexFormatInfo_s {
    VertexFormat vertex_format;
    char name[MAX_VERTEX_FORMAT_NAME_LENGTH];
    char attribute_name_data[ATTRIBUTE_NAME_DATA_LENGTH];
    int num_attributes;
    int attribute_name_indices[NUM_ATTRIBUTE_TYPES];
    bool attribute_types[NUM_ATTRIBUTE_TYPES]; // this is where attribute types slot in
    GLenum gl_types[NUM_ATTRIBUTE_TYPES];
    GLint gl_sizes[NUM_ATTRIBUTE_TYPES];
} VertexFormatInfo;

enum ShaderType {
    Vertex,
    Fragment,
    Geometry,
    TesselationControl,
    TesselationEvaluation,
    NUM_SHADER_TYPES
};

/* "Uniforms" as a structure encapsulate the association of a uniform name, location,
 * and method of geting it for upload, which is attached to a renderer structure, so the renderer can
 * update the values of uniform variables relevant to the shader program it is using.
 *
 * Typing is handled with a case-by-case matching to GL types, to use the correct GL functions
 * for uploading this type of uniform value.
 */
typedef union UniformData_union {
    GLuint int_value; //--- unsigned int. differentiate between these.
    GLfloat float_value;
} UniformData;
#define MAX_UNIFORM_NAME_LENGTH 32
typedef struct Uniform_s {
    char name[MAX_UNIFORM_NAME_LENGTH];
    GLuint location;
    GLuint type;
    UniformData (*get_uniform_value)(void);
} Uniform;

/* A "renderer" associates shaders for each shader type,
 * uniform variables to be updated when using the program linked from them,
 * and information enough to recompile these shaders into a new program.
 *
 * Rendering parameters and standard uniform variables are included in this structure.
 *
 * NOTES:
 *  I don't think this structure should contain information about the actual transformations
 *  used. So, the vertex shader will (probably) use MVP, yet the projection matrix could be
 *  given by a Camera structure, etc. So a "renderer" is just an association for shader program information.
 */
#define MAX_RENDERER_UNIFORMS 8
typedef struct Renderer_s {
    VertexFormat vertex_format;

    int num_uniforms;
    Uniform uniforms[MAX_RENDERER_UNIFORMS];

    GLuint program;
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint geometry_shader;
    GLuint tesselation_control_shader;
    GLuint tesselation_evaluation_shader;
    char *vertex_shader_path;
    char *fragment_shader_path;
    char *geometry_shader_path;
    char *tesselation_control_shader_path;
    char *tesselation_evaluation_shader_path;
} Renderer;

/* A "mesh" is a structure around the data in application memory. This is intended to be filled
 * with reading/generation routines, then freed from application memory after uploading to the
 * graphics server.
 */
typedef struct Mesh_s {
    VertexFormat vertex_format;
    unsigned int num_vertices;
    void **attribute_data[NUM_ATTRIBUTE_TYPES];
    AttributeType attribute_types[NUM_ATTRIBUTE_TYPES];
    unsigned int num_triangles;
    unsigned int *triangles;
} Mesh;

/* A "mesh handle" contains information to handle a single "mesh" entity
 * which is in graphics memory. This structure contains information enough
 * to free this mesh from graphics memory and call GL functions involving
 * this mesh data.
 */
typedef struct MeshHandle_s {
    //- if want to subdata a buffer that contains all of this, maybe have generic attached vbos which will be owned by this.
    VertexFormat vertex_format;
    GLuint vao;
    unsigned int num_vertices;
    GLuint attribute_vbos[NUM_ATTRIBUTE_TYPES];
    AttributeType attribute_types[NUM_ATTRIBUTE_TYPES];
    unsigned int num_triangles;
    GLuint triangles_vbo;
} MeshHandle;


//================================================================================
// Basic usage
//================================================================================
// Initialize a new renderer structure which only uses vertex and fragment shaders.
    void new_renderer_vertex_fragment(Renderer *renderer, char *vertex_shader_path, char *fragment_shader_path);
// Binding a renderer sets up the associated shader program and uniforms, ... so that GL draw calls will use them.
    void bind_renderer(Renderer *renderer);
// Dynamic recompilation (e.g. for testing, maybe elsewhere)
    void renderer_recompile_shaders(Renderer *renderer);

//================================================================================
// Uniforms
//================================================================================
// Associate a uniform to a renderer. It will be made sure to be uploaded before every draw with this renderer bound.
    Uniform *renderer_add_uniform(Renderer *renderer, char *name, UniformData (*get_uniform_value)(void), GLuint uniform_type);
// Upload the current values of the uniforms associated to the given renderer.
    void renderer_upload_uniforms(Renderer *renderer);

//================================================================================
// Vertex formats
//================================================================================
void init_vertex_formats(void);

//================================================================================
// Mesh and drawing functions
//================================================================================
// Free mesh from application memory.
    void free_mesh(Mesh *mesh);
// Upload mesh to graphics memory and initialize a mesh handle structure, and free mesh data from application memory.
    void upload_and_free_mesh(MeshHandle *mesh_handle, Mesh *mesh);
// Render a mesh associated to a mesh handle using the given renderer.
    void render_mesh(Renderer *renderer, MeshHandle *mesh_handle);

//================================================================================
// Printing and serialization
//================================================================================
    void print_renderer(Renderer *renderer);
    void print_mesh_handle(MeshHandle *mesh_handle);
    void serialize_mesh_handle(FILE *file, MeshHandle *mesh_handle);
    void print_vertex_formats(void);

//================================================================================
// Helper functions
//================================================================================
// Empty initialization of a renderer structure.
    void zero_init_renderer(Renderer *renderer);
// Helper functions to select shader entries in the renderer structure e.g. iteratively.
    GLuint renderer_shader_of_type(Renderer *renderer, int shader_type);
    char *renderer_shader_path_of_type(Renderer *renderer, int shader_type);


#endif // HEADER_DEFINED_MESH
