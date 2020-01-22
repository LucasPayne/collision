/*================================================================================
    Resources, resource management, and resource loading module.
================================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "helper_definitions.h"
#include "resources.h"
#include "data_dictionary.h"
#include "memory.h"

// The global resource dictionary is a data-dictionary intended to be set by the application and used when searching for resources.
// note: This is up to the load function for a resource. For example, a resource path may be interpreted as a physical path mapped as an asset/source file.
//        ---unsure if this is a good idea. Maybe rather everything should be in a .dd file, which would be less annoying with .dd generation.
DataDictionary *g_resource_dictionary = NULL;
#define RESOURCE_TABLE_SIZE 1024
ResourceTableEntry g_resource_table[RESOURCE_TABLE_SIZE];

// Static helper functions
// -----------------------
static ResourceID null_resource_id(void)
{
    ResourceID id;
    id.uuid = 0;
    id.type = 0;
    return id;
}

/*================================================================================
    Resource system initialization.
================================================================================*/
// Currently nothing needs to be done by the user.

/*================================================================================
    Resource types.
================================================================================*/
static int g_num_resource_types = 0;
ResourceTypeInfo *g_resource_type_info = NULL;
/*--------------------------------------------------------------------------------
Resource types are built up at runtime. This is so an application can include a call to a function for modules
which have different resource types to initialize these. Arbitrary resource types can then be introduced even at runtime. 
The resource type IDs are sorted out automatically, although they are not neccessarily consistent.

This function is non-static because a macro expands to it. Then
     add_resource_type(Texture)
expands to type metadata and passes in a global ResourceType pointer to Texture_RTID, gives it a size, the name of the structure,
and load function Texture_load. This allocates it an ID so further usage of macros which use the symbol Texture use this ID.
--------------------------------------------------------------------------------*/
void ___add_resource_type(ResourceType *type_pointer, size_t size, char *name, ResourceLoadFunction load, ResourceUnloadFunction unload)
{
    if (strlen(name) > MAX_RESOURCE_TYPE_NAME_LENGTH) {
        fprintf(stderr, ERROR_ALERT "Resource type name \"%s\" is too long. The maximum resource type name length is set to %d.\n", name, MAX_RESOURCE_TYPE_NAME_LENGTH);
        exit(EXIT_FAILURE);
    }
    ResourceTypeInfo *new_info;
    if (g_resource_type_info == NULL) {
        // Does realloc handle this anyway?
        g_num_resource_types = 1;
        g_resource_type_info = (ResourceTypeInfo *) calloc(1, sizeof(ResourceTypeInfo));
        mem_check(g_resource_type_info);
        new_info = &g_resource_type_info[0];
    } else {
        g_resource_type_info = (ResourceTypeInfo *) realloc(g_resource_type_info, (g_num_resource_types + 1) * sizeof(ResourceTypeInfo));
        mem_check(g_resource_type_info);
        new_info = &g_resource_type_info[g_num_resource_types]; // made room for it
        g_num_resource_types ++;
    }
    *type_pointer = g_num_resource_types - 1;
    new_info->type = g_num_resource_types - 1;
    new_info->size = size;
    new_info->load = load;
    new_info->unload = unload;
    strncpy(new_info->name, name, MAX_RESOURCE_TYPE_NAME_LENGTH);
}

/*================================================================================
    Resource handles, loading, and caching.
================================================================================*/

ResourceHandle ___new_resource_handle(ResourceType resource_type, char *path)
{
    // path-backed resource
    ResourceHandle resource_handle;
    resource_handle.path_backed = true;
    resource_handle._id = null_resource_id();
    resource_handle._id.type = resource_type;
    resource_handle.data.path = (char *) malloc((strlen(path) + 1) * sizeof(char));
    mem_check(resource_handle.data.path);
    strcpy(resource_handle.data.path, path);
    resource_handle._id.uuid = hash_crc32(resource_handle.data.path); // hopefully universally unique.
    return resource_handle;
}
void *___oneoff_resource(ResourceType resource_type, ResourceHandle *handle)
{
    // Create a oneoff, non-path-backed, non-shareable resource, set the resource handle
    handle->path_backed = false;
    handle->_id = null_resource_id();
    handle->_id.type = resource_type;
    handle->data.resource = calloc(1, g_resource_type_info[resource_type].size);
    mem_check(handle->data.resource);
    return handle->data.resource;
}
void *___resource_data(ResourceHandle *handle)
{
    if (!handle->path_backed) return handle->data.resource;
    // printf("Getting resource data from path %s...\n", handle->data.path);
    // printf("Handle:\n\tuuid: %u\n\ttype: %d\n", handle->_id.uuid, handle->_id.type);

    // note: Really this is "checking_entry" while the loop is running.
    ResourceTableEntry *new_entry = &g_resource_table[handle->_id.uuid % RESOURCE_TABLE_SIZE];
                                                    // At the end of search, if the resource wasn't found cached, this will be left as a pointer to fill with the new loaded entry.
                                                    // This complication is here since a new entry can be added either straight in the table, or at the end of one of the chains.

    #if 1 // set to 0  to force reload (and probably crash).
    if (new_entry->uuid != 0) {
        while (1) {
            if (new_entry->uuid == handle->_id.uuid) {
                // The point of this. Resource loading and unloading should be very rare compared to references to the resource,
                // so that should be a constant (-except chaining) fast lookup, yet still trigger a resource load if needed, unknown to the caller.
                // printf("Resource found cached.\n");
                return new_entry->resource;
            }
            if (new_entry->next == NULL) {
                // The end of the chain was reached, and no cached resource was found. Allocate a new entry and attach it to the end of the chain.
                new_entry->next = (ResourceTableEntry *) sma_alloc(sizeof(ResourceTableEntry)); // Using the small memory allocator here as well, to store the chains of the hash table.
                new_entry = new_entry->next;
                break;
            }
            new_entry = new_entry->next;
        }
    }
    #endif
    // printf("Resource not cached, loading ...\n");
    // The resource is not cached. Load it and cache it.
    ResourceTypeInfo *resource_type = &g_resource_type_info[handle->_id.type];
    void *resource = sma_alloc(resource_type->size); // Allocate it a block of an appropriate size using the small memory allocator.
    resource_type->load(resource, handle->data.path); // Use the relevant load function to fill the new resource data.
    new_entry->uuid = handle->_id.uuid;
    new_entry->type = handle->_id.type;
    new_entry->resource = resource;
    new_entry->next = NULL;
    return resource;
}


/*================================================================================
    Asset/source paths.
--- Completely redo this, make cleaner.
--- Change names to reflect these being assets and source paths for resources/manifests to reference,
    not to do with the data-dictionary resource paths.
================================================================================*/
/*
The resource path variable is created at runtime. This holds pairs, "drives" and their bound paths.
A resource path starts with a drive, probably capitalized. This can be bound to multiple actual directories
through the resource path variable. The resource path is resolved to find stuff for resource load/build.
Example:
     Project:/path/to/project:Application:/path/to/application:TextureLibrary:/path/to/texture/library:Project:alternate/project/directory

Now a resource path is stored on a resource handle,
     "TextureLibrary/dolphin"
This resolves to
      /path/to/texture/library/dolphin.
This may or may not be an actual file. It is not intended to be. The load behaviour of a resource type determines what is done with this path.
For example, a texture resource may be able to preconditioned, may have a ~manifest~ file describing what type of image file it is conditioned from,
have build directives and properties in the description file, etc.

The build/load process for a texture might be setup with
/path/to/texture/library/dolphin.Texture
                         dolphin.Texture.image
                         dolphin.png,            or, this idea being more useful, could be some sort of DCC format which needs a lot of preconditioning, like a psd.

dolphin.Texture:
    // A dolphin
    type: png
    source: dolphin.png
dolphin.Texture.image:
    Binary file. This is in a format very close to what the texture data actually is in application memory before vram upload.

The load function might check
    /path/to/texture/library/dolphin ---> /path/to/texture/library/dolphin.Texture.image,
and if it is there, maybe some validation, and this is loaded directly, possibly with some pointer swizzling, into memory
to be post-load conditioned, that is, uploaded to vram to a texture handle, which is the actual resource.

The conditioning/asset-compilation routine might be called as a backup if the "image" is not there, or it might fail (so the asset needs to be conditioned
before running the application), it might save the image file or it might not.
*/
static uint32_t g_resource_path_length = 0;
static char *g_resource_path = NULL;
int g_resource_path_count = 0;

void resource_path_add(char *drive_name, char *path)
{
    g_resource_path_count ++;

    // Append ":drive_name:path" to the resource path variable, and initialize/relocate if neccessary.
    size_t len = strlen(path) + 1 + strlen(drive_name);
    if (g_resource_path == NULL) {
        g_resource_path_length = len + 1;
        g_resource_path = (char *) malloc(g_resource_path_length * sizeof(char));
        mem_check(g_resource_path);
        strcpy(g_resource_path, drive_name);
        g_resource_path[strlen(drive_name)] = ':';
        strcpy(g_resource_path + strlen(drive_name) + 1, path);
        return;
    }
    uint32_t old_length = g_resource_path_length;
    g_resource_path_length += len + 1;
    g_resource_path = (char *) realloc(g_resource_path, g_resource_path_length * sizeof(char));
    mem_check(g_resource_path);
    g_resource_path[old_length - 1] = ':';
    strcpy(g_resource_path + old_length, drive_name);
    g_resource_path[old_length + strlen(drive_name)] = ':';
    strcpy(g_resource_path + old_length + strlen(drive_name) + 1, path);
}
/*
To use a resource path, suffixes are added to open related files.
Example:
    FILE *file = resource_file_open("TextureLibrary/dolphin", ".Texture", "rb");
will attempt to open, with a pair in the global path variable of TextureLibrary:/path/to/texture/library,
    "/path/to/texture/library/dolphin.Texture".

.Texture.image can be opened, manifest files read, build files like .png read, etc.
*/
FILE *resource_file_open(char *path, char *suffix, char *flags)
{
    char path_buffer[1024];
    for (int i = 0; i < g_resource_path_count; i++) {
        if (resource_file_path(path, suffix, path_buffer, 1024, i)) {
            FILE *file = fopen(path_buffer, flags);
            if (file != NULL) return file;
        }
    }
    return NULL;
}

bool resource_file_path(char *path, char *suffix, char *path_buffer, int path_buffer_size, int start_index)
{
    // --- This is not well thought out, and way too large a function for what it does. The start_index is a hack to
    // allow resource_file_open to fail constructing a path and continue looking for one.
    /* Returns whether or not the resource file path was successfully constructed. */
    char drive_buffer[64];
    char *drive = path;
    path = strchr(path, '/');
    if (path == NULL) {
        fprintf(stderr, "Bad path given.\n");
        exit(EXIT_FAILURE);
    }
    int drive_length = path - drive;
    char *prefix = g_resource_path;
    int up_to_path = 0;
    if (prefix == NULL) {
        printf("No resource path.\n");
        return NULL;
    }
    do {
        int i;
        for (i = 0; prefix[i] != ':'; i++) {
            if (prefix[i] == '\0') {
                fprintf(stderr, "Bad resource path variable.\n");
                exit(EXIT_FAILURE);
            }
            drive_buffer[i] = prefix[i];
        }
        drive_buffer[i] = '\0';

        i++;
        int j = 0;
        for (; prefix[i] != '\0' && prefix[i] != ':'; i++, j++) {
            if (j >= path_buffer_size) goto buffer_size_error;
            path_buffer[j] = prefix[i];
        }

        //----- check this
        path_buffer[j] = '/';
        if (strlen(path_buffer) + strlen(path) >= path_buffer_size) goto buffer_size_error;
        strcpy(path_buffer + j + 1, path);
        if (strlen(path_buffer) + strlen(suffix) >= path_buffer_size) goto buffer_size_error;
        strcpy(path_buffer + j + 1 + strlen(path), suffix);
    
        if (up_to_path >= start_index && strlen(drive_buffer) == drive_length && strncmp(drive_buffer, drive, drive_length) == 0) {
            // Checking lengths since a strncmp can have equal prefixes yet the drive buffer stores a longer drive name.
            return true;
        }
        prefix = strchr(prefix, ':');
        prefix = strchr(prefix + 1, ':');
        if (prefix != NULL) prefix ++;
        up_to_path ++;
    } while (prefix != NULL);
    return false;
buffer_size_error:
    fprintf(stderr, ERROR_ALERT "Resource path too large for given buffer.\n");
    exit(EXIT_FAILURE);
}
