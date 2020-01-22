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
File-backed resources described their source/assets with a "manifest", using the data
dictionary system. A path such as "Textures/minecraft/dirt" resolves to opening a global
resource dictionary, and following subdictionaries "Textures" and "minecraft", then subdictionary
"dirt", which is expected to be of a certain type, say a Texture dictionary. This is checked by the load function
for a Texture resource, which then extracts the Texture entries to load a resource, for example reading that the image is a png
and is sourced from a certain file.
--------------------------------------------------------------------------------*/
#include "data_dictionary.h"
// The global resource dictionary is a data-dictionary intended to be set by the application and used when searching for resources.
extern DataDictionary *g_resource_dictionary;

/*--------------------------------------------------------------------------------
    Resource IDs
--------------------------------------------------------------------------------
A resource ID is held by a resource handle. This gives an index into the resource
table for instant lookup, if it is there, a magic number ("uuid") for validation,
and a resource type for type-checking, if it needs to be done.
--------------------------------------------------------------------------------*/
typedef uint32_t ResourceUUID;
typedef uint32_t ResourceType;
typedef struct ResourceID_s {
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

This is a hash table so that resources can be cached, so that subsequent queries for
a resource with the same path retrieve the cached resource.
--------------------------------------------------------------------------------*/
#define RESOURCE_TABLE_START_SIZE 1024
typedef struct ResourceTableEntry_s {
    ResourceUUID uuid;
    ResourceType type;
    char *path;
    void *resource;
    struct ResourceTableEntry_s *next; //This struct is an entry in a chaining hash table.
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
    bool path_backed;
    ResourceID _id;
    union {
        // note: since path/resource memory is allocated, resource handles must be destroyed when the owner is destroyed/they are swapped.
        char *path; // for path-backed resources, for example ones that use the path to load the resource from a file.
        void *resource; // for non-path-backed resources, ones that hold and own the resource data itself.
    } data;
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

bool reload_resource(ResourceHandle *handle);

void *___oneoff_resource(ResourceType resource_type, ResourceHandle *handle);
#define oneoff_resource(RESOURCE_TYPE_NAME,RESOURCE_HANDLE)\
    ___oneoff_resource(( RESOURCE_TYPE_NAME ## _RTID ),\
                       &( RESOURCE_HANDLE ));

/*--------------------------------------------------------------------------------
    Resource types and the global resource type information array
--------------------------------------------------------------------------------
Resource type information is kept in a global dynamic array. This is built
up at runtime. A simple macro, used as "add_resource_type(Thing)",
expands to type information that will be put in the resource type info array.
The type ID is the index into this array.

The reason resource types work is that whenever a resource structure is declared,
in whatever module/application, it must be declared along with a global ResourceType "Thing_RTID",
RTID standing for resource type ID.  The add_resource_type macro expands to pass a
pointer to this global value to the routine which adds the new type. Now, macros can
use symbolic type names, because they expand to pass backend functions the global
Thing_RTID value, which is an index into the type info array.
--------------------------------------------------------------------------------*/
#define MAX_RESOURCE_TYPE_NAME_LENGTH 32
typedef struct ResourceTypeInfo_s {
    ResourceType type; // Its type is being used as its index in the global resource type info array.
    size_t size;
    char name[MAX_RESOURCE_TYPE_NAME_LENGTH + 1];
    void (*load) (void *resource, char *path);
    void (*unload) (void *resource);
} ResourceTypeInfo;
extern ResourceTypeInfo *g_resource_type_info;
void ___add_resource_type(ResourceType *type_pointer, size_t size, char *name, void *(*load)(char *), void (*unload)(void *resource));

// The resource's unload function does not free the allocated base resource structure. So,
// a resource may not need to do any more unloading such as freeing allocated memory.
// (and later, maybe dropping reference counts to handled resources.)
#define add_resource_type_no_unload(RESOURCE_TYPE_NAME)\
     ___add_resource_type(&( RESOURCE_TYPE_NAME ## _RTID ),\
                          sizeof(RESOURCE_TYPE_NAME),\
                          ( #RESOURCE_TYPE_NAME ),\
                          ( RESOURCE_TYPE_NAME ## _load ),\
                          NULL)
#define add_resource_type(RESOURCE_TYPE_NAME)\
     ___add_resource_type_no_unload(&( RESOURCE_TYPE_NAME ## _RTID ),\
                          sizeof(RESOURCE_TYPE_NAME),\
                          ( #RESOURCE_TYPE_NAME ),\
                          ( RESOURCE_TYPE_NAME ## _load ),\
                          ( RESOURCE_TYPE_NAME ## _unload ))

/*--------------------------------------------------------------------------------
    The "resource path variable" and drives
--------------------------------------------------------------------------------
A "path variable", similar to a Unix path variable e.g. "/path/to/bin/:/path/to/usr/bin/",
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
bool resource_file_path(char *path, char *suffix, char *path_buffer, int path_buffer_size, int start_index);
FILE *resource_file_open(char *path, char *suffix, char *flags);
void resource_path_add(char *drive_name, char *path);

// awful hack for help when opening resources ... "fix" for dropping out of path-search too early
extern int g_resource_path_count;

#endif // HEADER_DEFINED_RESOURCES
