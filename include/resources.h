/*================================================================================
    Resources, resource management, and resource loading module.
notes
--------------------------------------------------------------------------------


Some examples of ideas for how a resource system could be used

ResourceType Shader_RTID;
typedef struct Shader_s {
    GLuint vram_id;
} Shader;

----There is a serious problem with parameterized resources. Are they the same thing with different parameters? What is the "id" then? Can't just be the path.
----Doesn't seem useful.
To implement parameterized configurations, the resource type info struct could have
a union of function pointers and a flag for which type of call to use. Can functions pointers be cast
to a generic type so there are no type errors when macros expand to include login functions?

void login_shader(void *data, Stream *stream, int num_load_configs, char **load_configs)
{
    Shader *shader = (Shader *) data;

    GLenum shader_type = str_to_shader_type(get_config(load_configs, "shader_type"));
    GLuint vram_id = glCreateShader(shader_type);
    load_and_compile_shader(vram_id, stream); //?

    shader->vram_id = vram_id;
}
void logout_shader(void *data)
{
    Shader *shader = (Shader *) data;
    glDeleteShader(shader->vram_id);
}


ResourceType Renderer_RESOURCE_TYPE_ID;
typedef struct Renderer_s {
    VertexFormat vertex_format;

    int num_uniforms;
    Uniform uniforms[MAX_RENDERER_UNIFORMS];

    GLuint program_vram_id;

    ResourceHandle shaders[NUM_SHADER_TYPES];
} Renderer;
void load_renderer ...

ResourceType Model_RESOURCE_TYPE_ID;
// ResourceType Model_RTID;
typedef struct Model_s {
    ResourceHandle mesh; // MeshHandle
    ResourceHandle textures[12]; // TextureHandle
    ResourceHandle renderer; // Renderer
    int num_configs;
    char **model_config; // just as an example of a non-resource
} Model;
static void login_model(void* data, Stream *stream)
{
    // Space is already allocated on the heap.
    
    Model *model = (Model *) data;
    // Hand the model manifest to a lexer which fills the Model structure, due to a standard
    // file format for composite resources.
    read_model(model, stream); // fills in and allocates for model_config
}

static void logout_model(void *data)
{
    Model *model = (Model *) data;
    remove_resource(MeshHandle, model->mesh); // this handles reference counting, so need not actually unload.
    for(int i = 0; i < 12; i++) remove_resource(TextureHandle, model->textures[i]);
    remove_resource(Renderer, model->renderer);

    // Dynamic data is freed here.
    for (int i = 0; i < model->num_configs; i++) {
        free(model->model_config[i]);
    }
    free(model->model_config);

    // whatever calls logout_model will free the structure from the heap.
}

add_resource_type


Model *dolphin = load_resource(Model, "Project/models/dolphin.model");
render_model(dolphin, camera);
causes
Renderer *renderer = resource_data(Renderer, model->renderer);
causing path lookup, "Project/renderers/dolphin.renderer" (path loaded from whatever the .model file is)
adds resource, uses type Renderer_RESOURCE_TYPE_ID to lookup in the global resource type info table the loading
function, calls it with the renderer resource ID and resource handle's path, and this should load the renderer.
Subsequent calls need only cause this if the renderer is unloaded, or if it is moved (unloaded and loaded), a path
lookup is done but does not call the loading function.

This renderer has resource handles to shaders. Say there are only vertex and fragment shaders, then when they are needed
(probably very soon), this triggers again the call to load these resources from the paths stored in the mesh handles. Say the fragment shader
is a standard one and is already being used, then the path lookup resolves and the dereference (say in the first draw call to this renderer)
resolves to that fragment shader, already compiled in vram.

The model rendering also triggers resource loads for textures, say some are standard bump/lighting maps, and one is unique and unloaded, these
resolve in the same way.

So resources can be media files, small data structures, associations of resources, etc. Or, they can be small data structures that
are handles themselves e.g. for stuff in vram, so the resource will still want to be shared so it doesn't cause a reupload.


    // Game Engine Architecture p300
    // Function pointers for post-load initialization and tear down.
    // "Logging in" and "logging out" for resource types.
    // -----
    // load_and_compiler_shader
    // upload_and_free_mesh
    // ^ These functions already work, so the resource type wraps around these as the loader.
    // Then tear-down is simple in these cases, freeing the IDs with GL calls and doing a generic application-memory resource unload.
    // (so, could just free from the heap.)


================================================================================*/
#ifndef HEADER_DEFINED_RESOURCES
#define HEADER_DEFINED_RESOURCES

typedef uint64_t ResourceUUID;
typedef uint32_t ResourceType;

// A "null" resource ID or resource table entry has uuid zero. This is a macro for
// returning a null resource ID for errors.
typedef struct ResourceID_s {
    uint32_t table_index;
    ResourceUUID uuid;
    ResourceType type;
} ResourceID;

#define RESOURCE_TABLE_START_SIZE 1024
typedef struct ResourceTableEntry_s {
    ResourceUUID uuid;
    ResourceType type;
    char *path;
    void *resource;
} ResourceTableEntry;

typedef struct ResourceHandle_s {
    // Owner of this handle must not access these directly.
    ResourceID _id;
    char *_path;
    void *_parameters;
} ResourceHandle;

#define MAX_RESOURCE_TYPE_NAME_LENGTH 32
typedef struct ResourceTypeInfo_s {
    ResourceType type; // Its type is being used as its index in the global resource type info array.
    size_t size;
    char name[MAX_RESOURCE_TYPE_NAME_LENGTH + 1];
    void *(*load) (char *path);
    bool has_parameters;
    size_t parameters_size;
} ResourceTypeInfo;

#define MAX_RESOURCE_BRANCH_NAME_LENGTH 32
typedef struct ResourceTree_s {
    char name[MAX_RESOURCE_BRANCH_NAME_LENGTH + 1];
    struct ResourceTree_s *next;
    bool is_leaf;
    union contents_union {
        struct ResourceTree_s *first_child;
        ResourceID resource_id;
    } contents;
} ResourceTree;


/*
 * When a resource type is added, be it in a library module or in a specific application,
 * that module/application must define a global variable
 * ResourceType Name_RTID;
 * Then the typing macro for resource type addition expands to give a new resource type a unique identifier by passing this global's address, and further
 * typed macro usage expands to this type id to lookup type info in a global table updated from calls to create a new resource type.
 *
 * A resource is always a struct. The resource type name given is the name of the struct, and this macro
 * expands to give type information (unique id, size, type name for debugging).
 *  
 * Logging in and logging out out resources could by default not do anything past the generic application-memory struct allocation/deallocation.
 * If a resource owns things, handles to GL objects in vram, (? sub-resources (if reference counting is used ?)), or needs
 * conditioning (if the resource load is a runtime conditioning from a PLY file, then it needs to be done rather than
 * just directly loading a binary image of the wanted structure.)
 *
 * The macro could just expand to name_login and name_logout functions that must be defined, then
 * puts it in the resource type info structure.
 */

#define add_resource_type(RESOURCE_TYPE_NAME)\
     ___add_resource_type(&( RESOURCE_TYPE_NAME ## _RTID ),\
                          sizeof(RESOURCE_TYPE_NAME),\
                          ( #RESOURCE_TYPE_NAME ),\
                          ( RESOURCE_TYPE_NAME ## _load ))

#define add_resource_parameters(RESOURCE_TYPE_NAME)\
    ___add_resource_parameters(( RESOURCE_TYPE_NAME ## _RTID ),\
                               sizeof( RESOURCE_TYPE_NAME ## Parameters ))

#define resource_data(STRUCTURE_NAME,HANDLE)\
    ( (STRUCTURE_NAME *) ___resource_data( &( HANDLE ) ) )

#define new_resource_handle(RESOURCE_TYPE_NAME,PATH)\
    ___new_resource_handle(( RESOURCE_TYPE_NAME ## _RTID),\
                           ( PATH ))

////// Should be "update", because this should _not_ be used to create a new resource handle, rather replace it.
#define init_resource_handle(RESOURCE_TYPE_NAME,RESOURCE_HANDLE,PATH)\
    ___init_resource_handle(( RESOURCE_TYPE_NAME ## _RTID),\
                            &( RESOURCE_HANDLE ),\
                            ( PATH ))

// These functions are non-static because type-name to type-id macros resolve to them.
void ___add_resource_type(ResourceType *type_pointer, size_t size, char *name, void *(*load)(char *));
void *___resource_data(ResourceHandle *handle);
ResourceHandle ___new_resource_handle(ResourceType resource_type, char *path);
void ___init_resource_handle(ResourceType resource_type, ResourceHandle *resource_handle, char *path);

// Paths and search
//================================================================================
bool resource_file_path(char *path, char *suffix, char *path_buffer, int path_buffer_size);
FILE *resource_file_open(char *path, char *suffix, char *flags);
void resource_path_add(char *drive_name, char *path);
ResourceID add_resource(ResourceID id, char *path);
ResourceID lookup_resource(char *path);

// Testing
//================================================================================
void test_resource_tree(void);

// Helper functions and printing
//================================================================================
void print_resource_tree(void);
void print_resource_types(void);
void print_resource_path(void);


#endif // HEADER_DEFINED_RESOURCES
