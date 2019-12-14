/*================================================================================
    Resources, resource management, and resource loading module.
================================================================================*/
#ifndef HEADER_DEFINED_RESOURCES
#define HEADER_DEFINED_RESOURCES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
/*--------------------------------------------------------------------------------
    Resource IDs
--------------------------------------------------------------------------------
A resource ID is held by a resource handle. This gives an index into the resource
table for instant lookup, if it is there, a magic number ("uuid") for validation,
and a resource type for type-checking, if it needs to be done.
--------------------------------------------------------------------------------*/
typedef uint64_t ResourceUUID;
typedef uint32_t ResourceType;
typedef struct ResourceID_s {
    uint32_t table_index;
    ResourceUUID uuid;
    ResourceType type;
} ResourceID;

/*--------------------------------------------------------------------------------
    The global resource table 
--------------------------------------------------------------------------------
Active resource information is stored in a global resource table, a dynamic
array of ResourceTableEntry structs. These don't contain resource IDs, but
a magic number ("uuid") for validating instant resource lookups, a type
for type-checking, a path to facilitate reloading and querying, and
a pointer to the actual resource, wherever it is.
--------------------------------------------------------------------------------*/
#define RESOURCE_TABLE_START_SIZE 1024
typedef struct ResourceTableEntry_s {
    ResourceUUID uuid;
    ResourceType type;
    char *path;
    void *resource;
} ResourceTableEntry;
extern ResourceTableEntry *g_resource_table;

/*--------------------------------------------------------------------------------
    Resource handles and resource "dereferencing"
--------------------------------------------------------------------------------
The point of the resource system is to facilitate object sharing and automatic
loading and management. So, instead of holding a direct structure or a raw pointer,
something that wants to use a resource holds a ResourceHandle. A collection
of macros allows the user to "dereference" this resource handle, which behind
the scenes may instantly get the resource from the global table, query
the path to see if it is already loaded, or actually trigger a resource load,
and update the identifier contained in the handle. It is a sort of "smart
reference" to a resource.
--------------------------------------------------------------------------------*/
typedef struct ResourceHandle_s {
    // Owner of this handle must not access these directly. However, resource
    // managing functions will need to.
    ResourceID _id;
    char *_path;
} ResourceHandle;

void *___resource_data(ResourceHandle *handle);
ResourceHandle ___new_resource_handle(ResourceType resource_type, char *path);
void ___init_resource_handle(ResourceType resource_type, ResourceHandle *resource_handle, char *path);

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

/*--------------------------------------------------------------------------------
    Resource types and the global resource type information array
--------------------------------------------------------------------------------
Resource type information is kept in a global dynamic array. This is built
up at runtime. A simple macro, used as "add_resource_type(Thing)",
expands to type information that will be put in the resource type info array.
The type ID is the index into this array.

The reason resource types work is that whenever a resource structure is declared,
in whatever module/application, it must be declared along with a global "Thing_RTID",
RTID standing for resource type ID.  The add resource type macro expands to pass a
pointer to this global value to the routine which adds the new type. Now, macros can
use symbolic type names, because they expand to pass backend functions the global
Thing_RTID value, which is an index into the type info array.
--------------------------------------------------------------------------------*/
#define MAX_RESOURCE_TYPE_NAME_LENGTH 32
typedef struct ResourceTypeInfo_s {
    ResourceType type; // Its type is being used as its index in the global resource type info array.
    size_t size;
    char name[MAX_RESOURCE_TYPE_NAME_LENGTH + 1];
    void *(*load) (char *path);
} ResourceTypeInfo;
extern ResourceTypeInfo *g_resource_type_info;
void ___add_resource_type(ResourceType *type_pointer, size_t size, char *name, void *(*load)(char *));
#define add_resource_type(RESOURCE_TYPE_NAME)\
     ___add_resource_type(&( RESOURCE_TYPE_NAME ## _RTID ),\
                          sizeof(RESOURCE_TYPE_NAME),\
                          ( #RESOURCE_TYPE_NAME ),\
                          ( RESOURCE_TYPE_NAME ## _load ))

/*--------------------------------------------------------------------------------
    The "resource tree" and resource-path querying
--------------------------------------------------------------------------------
Resources' "real" globally unique identifier is the "path" they are created with.
These paths can be used to query whether or not a resource is loaded. This is
(currently, it does not seem too good) done by maintaining a tree which the
query follows by the path's entries, e.g. "Shaders/texturing/test.frag" follows the tree
as (root) --> Shaders --> texturing --> test.frag.

If this gets to a leaf, then the leaf contains the resource type ID, including the index
into the global resource table. So, this is used so that resource handle "dereferences"
synchonize to the correct ID if the resource is already loaded, and subsequent dereferences
will use that ID.

If the path can't be followed, then the resource isn't loaded. Resources are added
to the tree by filling out the tree just enough to store the resource ID at the leaf.
--------------------------------------------------------------------------------*/
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

ResourceID add_resource(ResourceID id, char *path);
ResourceID lookup_resource(char *path);

/*--------------------------------------------------------------------------------
    The "resource path variable" and drives
--------------------------------------------------------------------------------
A "path variable" similar to a Unix path variable e.g. "/path/to/bin/:/path/to/usr/bin/",
is kept which holds maps between "resource drives" and actual directories.

The resource path variable is built up at runtime through the function
resource_path_add. A path's first entry is its "resource drive", so things
which work with resource paths try to map this to a drive-directory pair using
the resource path variable.
If the resource path variable is
    "Shaders:/path/to/project/assets/shaders:Textures:/path/to/project/assets/textures"
then Textures/blocks/dirt maps to "/path/to/project/assets/textures/dirt". This
may not be an actual file, but can be used as the base for an asset loading
routine to add suffixes to find relevant asset files (such as adding a .png, that failing,
adding a .bmp, etc.)
--------------------------------------------------------------------------------*/
bool resource_file_path(char *path, char *suffix, char *path_buffer, int path_buffer_size);
FILE *resource_file_open(char *path, char *suffix, char *flags);
void resource_path_add(char *drive_name, char *path);

// Testing
//================================================================================
void test_resource_tree(void);

// Helper functions and printing
//================================================================================
void print_resource_tree(void);
void print_resource_types(void);
void print_resource_path(void);

#endif // HEADER_DEFINED_RESOURCES
