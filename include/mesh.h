/*================================================================================
   Mesh and mesh rendering module.

   OpenGL state and capabilities are encapsulated here by structures.

   "renderers":
       Higher level shader programs. This structure gives information enough to recompile a shader program
       consisting of different shader stages. Creating a new renderer loads and compiles
       the shaders and associates them to the structure so they can later be bound for drawing,
       recompiled, and freed. "Binding" a renderer prepares draw calls at the level of this module
       to use that renderer and its associated shaders.
       Shader input:
       Uniforms:
            Uniform input to the shaders associated to a renderer are encapsulated in this structure
            by a Uniform structure array. "Uniforms" here give the renderer enough information to find the value of
            the uniform and upload it correctly. This is done when a renderer is bound, so GL draw calls will
            use the uniform values calculated with these associated functions.
       Vertex formats:
            A renderer has a single bitmask denoting its vertex format. Each entry corresponds
            to a vertex attribute, and draw calls will compare bitmasks to see if the vertex
            formats are compatible (that is, there is enough data associated to a mesh to be
            rendered with this renderer. Extra data is ignored.)

    "meshes and mesh handles":
        Here, a "mesh" is a Mesh structure, encapsulating mesh data in application memory.
        A "mesh handle" is a MeshHandle structure, encapsulating mesh data in video memory after it
        has been uploaded. Functions are provided to upload mesh data to VRAM, and do not neccessarily
        free the application data. This data can be freed once a mesh handle is created/filled, and
        can be used with this module's draw functions to render the mesh from VRAM with a given renderer.

    Mesh loading is also included here. This should be moved to another module such as mesh/ply.
    File formats:
        PLY (The Stanford triangle format)

================================================================================*/
#ifndef HEADER_DEFINED_MESH
#define HEADER_DEFINED_MESH
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdint.h>
#include "helper_definitions.h"
#include "helper_gl.h"
#include "ply.h"

typedef uint8_t AttributeType;
// Corresponds to layout qualified positions in shaders
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

// Vertex formats are bitmasks.
// Attribute types have a flag position left from the least significant bit by the value of the
// attribute type. So, the value of an attribute type:
//  Associates it to an AttributeInfo structure in a global array.
//  Associates it to a bitmask which can be bitwise OR'd to create vertex formats.
//  Indexes into any array which holds information to do with attributes.
//      For example, a mesh handle's vbo id for the attribute. The other entries should be nulled/not checked
//      if their corresponding bit in the held vertex format of whatever structure is using vertex attributes is not 1.
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

enum ShaderType {
    Vertex,
    Fragment,
    Geometry,
    TesselationControl,
    TesselationEvaluation,
    NUM_SHADER_TYPES
};

typedef struct ShaderID_s {
    uint32_t table_index;
    uint32_t uuid; // 32 bits probably fine
} ShaderID;
#define NULL_SHADER_ID { 0, 0 }
#define MAX_SHADER_NAME_LENGTH 32;
typedef struct Shader_s {
    ShaderID id;
    char *hash_path;
    char *resource_path;
    GLuint vram_id;
} Shader;

#define SHADER_TABLE_START_SIZE 128
static uint32_t g_last_shader_uuid = 0;
static uint32_t g_shader_table_size = SHADER_TABLE_START_SIZE;
static bool g_shader_table_initialized = false;
static Shader **g_shader_table;

ShaderID create_shader_id(void)
{
    if (!g_shader_table_initialized) {
        g_shader_table_size = SHADER_TABLE_START_SIZE;
        g_shader_table = (Shader **) calloc(1, sizeof(Shader *) * g_shader_table_size);
        mem_check(g_shader_table);
        g_shader_table_initialized = true;
    }

    int i = 0;
    while (1) {
        for (; i < g_shader_table_size; i++) {
            if (g_shader_table[i] == NULL) {
                ShaderID new_id;
                new_id.table_index = i;
                new_id.uuid = ++g_last_shader_uuid;
                return new_id;
            }
        }
        uint32_t previous_size = g_shader_table_size;
        g_shader_table_size += SHADER_TABLE_START_SIZE; // grow the shader table linearly.
        g_shader_table = (Shader **) realloc(g_shader_table, sizeof(Shader *) * g_shader_table_size);
        mem_check(g_shader_table);
        // Null initialize the new available shader pointers.
        memset(g_shader_table + previous_size, 0, sizeof(Shader *) * (g_shader_table_size - previous_size));
    }
}

Shader *create_shader(char *hash_path, char *resource_path, GLenum shader_type)
{
    // Load and compile the shader module in vram. vram_id will be given to the new shader.
    vram_id = glCreateShader(shader_type);
    load_and_compile_shader(vram_id, resource_path);

    // Create a new shader on the heap and set the table pointer.
    ShaderID id = create_shader_id();
    g_shader_table[id.table_index] = (Shader *) calloc(1, sizeof(Shader));
    mem_check(g_shader_table[id.table_index]);
    Shader *new_shader = g_shader_table[id.table_index];

    // Allocate memory for paths and give them to the new shader.
    new_shader->hash_path = (char *) malloc((strlen(hash_path) + 1) * sizeof(char));
    mem_check(new_shader->hash_path);
    strcpy(new_shader->hash_path, hash_path);
    new_shader->resource_path = (char *) malloc((strlen(resource_path) + 1) * sizeof(char));
    mem_check(new_shader->resource_path);
    strcpy(new_shader->resource_path, resource_path);

    // Give ids to the new shader.
    new_shader->id = id;
    new_shader->vram_id = vram_id;

    return new_shader;
}


/* "Uniforms" as a structure encapsulate the association of a uniform name, location,
 * and method of geting it for upload, which is attached to a renderer structure, so the renderer can
 * update the values of uniform variables relevant to the shader program it is using.
 *
 * Typing is handled with a case-by-case matching to GL types, to use the correct GL functions
 * for uploading this type of uniform value.
 */
// I don't think GL has matrix types, so using these types for uniforms.
typedef uint8_t UniformType;
enum UniformTypes {
    UNIFORM_FLOAT,
    UNIFORM_INT,
    UNIFORM_MAT4X4
};
typedef union UniformData_union {
    GLuint int_value; //--- unsigned int. differentiate between these.
    GLfloat float_value;
    GLfloat mat4x4_value[4 * 4];
} UniformData;
#define MAX_UNIFORM_NAME_LENGTH 32
typedef struct Uniform_s {
    char name[MAX_UNIFORM_NAME_LENGTH + 1];
    GLuint location;
    UniformType type;
    UniformData (*get_uniform_value)(void);
} Uniform;

/* A "renderer" associates shaders for each shader type,
 * uniform variables to be updated when using the program linked from them,
 * and information enough to recompile these shaders into a new program.
 */
#define MAX_RENDERER_UNIFORMS 8
typedef struct Renderer_s {
    VertexFormat vertex_format;

    int num_uniforms;
    Uniform uniforms[MAX_RENDERER_UNIFORMS];

    GLuint program_vram_id;

    ShaderID shaders[NUM_SHADER_TYPES];
} Renderer;

/* A "mesh" is a structure around the data in application memory. This is intended to be filled
 * with reading/generation routines, then freed from application memory after uploading to the
 * graphics server.
 */
typedef struct Mesh_s {
    VertexFormat vertex_format;
    unsigned int num_vertices;
    void *attribute_data[NUM_ATTRIBUTE_TYPES];
    unsigned int num_triangles;
    unsigned int *triangles; //----change to exact types, uint32_t
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
    unsigned int num_triangles;
    GLuint triangles_vbo;
} MeshHandle;


//================================================================================
// Basic usage
//================================================================================
// Initialize a new renderer structure which only uses vertex and fragment shaders.
    void new_renderer_vertex_fragment(Renderer *renderer, VertexFormat vertex_format, char *vertex_shader_path, char *fragment_shader_path);
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
    void print_vertex_attribute_types(void);

//================================================================================
// Mesh and drawing functions
//================================================================================
// Free mesh from application memory.
    void free_mesh(Mesh *mesh);
// Upload mesh to graphics memory and initialize a mesh handle structure
    void upload_mesh(MeshHandle *mesh_handle, Mesh *mesh);
// + free mesh from application memory (if it was malloc'd)
    void upload_and_free_mesh(MeshHandle *mesh_handle, Mesh *mesh);
// Render a mesh associated to a mesh handle using the given renderer.
    void render_mesh(Renderer *renderer, MeshHandle *mesh_handle, GLenum primitive_mode);

//================================================================================
// Printing and serialization
//================================================================================
    void print_renderer(Renderer *renderer);
    void print_mesh_handle(MeshHandle *mesh_handle);
    void serialize_mesh_handle(FILE *file, MeshHandle *mesh_handle);
    void print_mesh(Mesh *mesh);
    void print_vertex_format(VertexFormat vertex_format);
    void fprint_vertex_format(FILE *file, VertexFormat vertex_format); //--- do fprints for everything

//================================================================================
// Helper functions
//================================================================================
// Empty initialization of a renderer structure.
    void zero_init_renderer(Renderer *renderer);
// Helper functions to select shader entries in the renderer structure e.g. iteratively.
    GLuint renderer_shader_of_type(Renderer *renderer, int shader_type);
    char *renderer_shader_path_of_type(Renderer *renderer, int shader_type);

//================================================================================
// Loaders
// notes:
//      Maybe should remove this functionality from here.
//================================================================================
void load_mesh_ply(Mesh *mesh, VertexFormat vertex_format, char *ply_filename, float size_factor);


#endif // HEADER_DEFINED_MESH
