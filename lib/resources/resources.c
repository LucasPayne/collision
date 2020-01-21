/*--------------------------------------------------------------------------------
    Resources, resource management, and resource loading module.

notes:
   Switched to chaining hash table. The uuid is now a cyclic-redundancy-check 32 bit int.
   Jason Gregory says that no clash was found at naughty dog during development. It probably will not happen.

--------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "helper_definitions.h"
#include "resources.h"
#include "data_dictionary.h"

// The global resource dictionary is a data-dictionary intended to be set by the application and used when searching for resources.
DataDictionary *g_resource_dictionary = NULL;

//--------------------------------------------------------------------------------
// Static function declarations
//--------------------------------------------------------------------------------
static ResourceID create_resource_id(ResourceType type);
static ResourceHandle *relocate_resource_handle(ResourceHandle *resource_handle);
static ResourceID null_resource_id(void);

//--------------------------------------------------------------------------------
// Resource types.
static int g_num_resource_types = 0;
ResourceTypeInfo *g_resource_type_info = NULL;

// Resource table and ID allocation.
// This also acts as a cache.
static bool g_initialized_resource_table;
static uint32_t g_resource_table_size = 0;
ResourceTableEntry *g_resource_table;
static ResourceUUID g_last_resource_uuid = 0;

// Asset/source paths and search.
static uint32_t g_resource_path_length = 0;
static char *g_resource_path = NULL;
int g_resource_path_count = 0;


/*
Resource types are built up at runtime. This is so an application can include a call to a function for modules
which have different resource types to initialize these. Arbitrary resource types can then be introduced even at runtime. 
The resource type IDs are sorted out automatically, although they are not neccessarily consistent.

This function is non-static because a macro expands to it. Then
     add_resource_type(Texture)
expands to type metadata and passes in a global ResourceType pointer to Texture_RTID, gives it a size, the name of the structure,
and load function Texture_load. This allocates it an ID so further usage of macros which use the symbol Texture use this ID.
 */
void ___add_resource_type(ResourceType *type_pointer, size_t size, char *name, void *(*load)(char *), void (*unload)(void *resource))
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
/*
This gives a new resource ID which should be used instantly after creation. When returned, this is a usable ID, so
other functions (probably just the one that uses this) can create the new resource entry at the given table index
and with the allocated uuid.
*/
// static ResourceID create_resource_id(ResourceType type)
// {
//     if (!g_initialized_resource_table) {
//         g_resource_table_size = RESOURCE_TABLE_START_SIZE;
//         g_resource_table = (ResourceTableEntry *) calloc(1, sizeof(ResourceTableEntry) * g_resource_table_size);
//         mem_check(g_resource_table);
//         g_initialized_resource_table = true;
//     }
//     int i = 0;
//     while (1) {
//         // for (; i < g_resource_table_size; i++) {
//         //     if (g_resource_table[i].uuid == 0) {
//         //         ResourceID new_id;
//         //         new_id.table_index = i;
//         //         new_id.uuid = ++g_last_resource_uuid;
//         //         new_id.type = type;
//         //         return new_id;
//         //     }
//         // }
//         // uint32_t previous_size = g_resource_table_size;
//         // g_resource_table_size += RESOURCE_TABLE_START_SIZE; // grow the resource table linearly.
//         // g_resource_table = (ResourceTableEntry *) realloc(g_resource_table, sizeof(ResourceTableEntry) * g_resource_table_size);
//         // mem_check(g_resource_table);
//         // // Null initialize the new available resource table entries.
//         // memset(g_resource_table + previous_size, 0, sizeof(ResourceTableEntry) * (g_resource_table_size - previous_size));
//     }
// }

/*
This function is non-static because a macro expands to it. This gets the pointed to, raw resource
which a resource handle references. This "dereferences"/relocates the resource handle, so it
a valid handle (if it needs to load, then if the resource exists and can load), which must be done every time a resource
is used.
notes:
     Since everything is synchronous so far, the caller can safely use this data until a set time (say per frame)
     when resources are allowed to be removed or relocated. However, if more complicated memory management stuff is added that
     doesn't restrict to per frame (done asynchronously), resource locking will be needed.
*/
void *___resource_data(ResourceHandle *handle)
{
    /* if (type != handle._id.type) { */
    /*     fprintf(stderr, ERROR_ALERT "Type error when dereferencing resource handle.\n"); */
    /*     exit(EXIT_FAILURE); */
    /* } */
    if (!handle->path_backed) {
        // When the resource handle is not backed by a path (for example when the resource is loaded from a file),
        // then it is a oneoff resource, and the resource data is owned by the handle, storing it where it would have stored the path.
        return handle->data.resource;
    }
    // The resource is path-backed, so relocate it as usual, possibly triggering a resource load.
    relocate_resource_handle(handle);
    if (handle->_id.uuid == 0) {
        fprintf(stderr, ERROR_ALERT "Could not relocate resource handle.\n");
        exit(EXIT_FAILURE);
    }
    /* printf("Got resource in table, %s, ", g_resource_table[handle->_id.table_index].path); */
    /* printf("with uuid %ld\n", g_resource_table[handle->_id.table_index].uuid); */

    return g_resource_table[handle->_id.table_index].resource;
}

/*
When a resource handle is initialized, its id is null. This will cause a path lookup
when the handle is dereferenced, possibly loading the resource, or just giving the handle
the (for now) valid id.

////// Should be "update", because this should _not_ be used to create a new resource handle, rather replace it.

This function is static because a type-symbol macro expands to it.
 */
void ___init_resource_handle(ResourceType resource_type, ResourceHandle *resource_handle, char *path)
{
    // This function can be used to reset a resource handle to something else, so free the stored path if needed.
    if (resource_handle->data.path != NULL) free(resource_handle->data.path);
    resource_handle->_id = null_resource_id();
    resource_handle->_id.type = resource_type;
    resource_handle->data.path = (char *) malloc((strlen(path) + 1) * sizeof(char));
    mem_check(resource_handle->data.path);
    strcpy(resource_handle->data.path, path);
}
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
    return resource_handle;
}

// When a resource is not file-backed or not intended to be shared, it is a oneoff resource. This is still a ResourceHandle
// so the sharing mechanisms can be available, for example for a Model object which may have its mesh procedurally defined,
// or backed by a file with a resource path, etc., it would be wanted that the resource system just handles file-backed loading,
// virtual-resource sharing, and oneoff resource creation, in a way where the application can just use the resource without thinking
// about what is going to be shared or whether or not it was loaded from a file.
//
// There could also be a "virtual resource" which is procedurally/alternatively defined, yet still is intended to be shared. This would
// use the same GUID-as-resource-paths system, except, for example, prepend the paths by "Virtual/" and make sure they don't clash with the file-backed
// resources. Then, just initialize the resource before the resource system tries to actually load it from this "Virtual/..." path. Then things with the
// same "Virtual/..." name will be shared. A oneoff resource is not shareable as it does not have a name.
// -------------
// Example usage
// Geometry *g = oneoff_resource(Geometry, body->geometry);
// gm_triangles(VERTEX_FORMAT_3);
// ... set some stuff
// *g = gm_done();
//
// Consider not using a oneoff resource and just creating a Geometry struct, if it is not needed to be stored in something which holds
// a resource handle, for example when rendering procedural transient geometry. In that case the resourceness of Geometry is irrelevant.
//
// whatever uses the "body" object doesn't care if the geometry is loaded from a file,
// a shared virtual resource, or procedurally generated and not backed by a path, so non-shareable. It just
// uses the geometry.
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

/*
The caller should not know or care whether a dereference caused a relocation.
The dereference should be safe for the function using it but shouldn't be saved. The pointer is returned back only for
the convenience of relocating/validating and using an attribute in the same expression.
*/
static ResourceHandle *relocate_resource_handle(ResourceHandle *resource_handle)
{
    /* This function is only called when accessing resource data for a path-backed resource.
     */
#define DEBUG 0
    // If the resource handle's id is null, then a lookup needs to be done first instead of trying the table.
    if (resource_handle->_id.uuid != 0) {
        // Table lookup the resource. The null uuid is 0, so an empty entry cannot be matched.
        // Check the uuids of all entries in a chain in the hash table.
	ResourceTableEntry *checking_entry = &g_resource_table[resource_handle->_id.table_index];
        while (checking_entry != NULL) {
            checking_entry = checking_entry->next;
            if (strcmp(checking_entry->path, resource_handle->data.path) == 0) { //slow, see below.
            // if (checking_entry->uuid == resource_handle->_id.uuid) { //-----much faster. uncomment when crc32 is implemented.
                // The point of this. Resource loading and unloading should be very rare compared to references to the resource,
                // so that should be a constant (-except chaining) fast lookup, yet still trigger a resource load if needed, unknown to the caller.
                if (DEBUG) printf("Handle dereferenced straight to the resource table.\n");
                return resource_handle;
            }
        }
    }

    if (resource_handle->data.path == NULL) {
        fprintf(stderr, ERROR_ALERT "Attempted to dereference resource handle which has no resource path.\n");
        exit(EXIT_FAILURE);
    }
    // The resource isn't loaded. Trigger a load.
    // First, create a new entry in the resource table with a new ID.
    if (!g_initialized_resource_table) {
        g_resource_table_size = RESOURCE_TABLE_START_SIZE;
        g_resource_table = (ResourceTableEntry *) calloc(1, sizeof(ResourceTableEntry) * g_resource_table_size);
        mem_check(g_resource_table);
        g_initialized_resource_table = true;
    }
    ResourceID id;
    id.uuid = hash_crc32(resource_handle->data.path);
    id.type = resource_handle->_id.type;
    id.table_index = id.uuid % g_resource_table_size;

    // Insert the entry into the chaining hash table.
    ResourceTableEntry *new_entry = &g_resource_table[id.table_index];
    bool already_cached = false;
    if (new_entry->uuid != 0) { //initial cell is occupied, chain into the table.
        while (1) {
            if (new_entry->uuid == 0) break; // found the end of the chain.
            if (new_entry->uuid == id.uuid) {
                // the resource is already cached.
                already_cached = true;
                break;
            }
            new_entry = new_entry->next;
        }
        if (!already_cached) {
            new_entry->next = (ResourceTableEntry *) calloc(1, sizeof(ResourceTableEntry));
            mem_check(new_entry->next);
            new_entry = new_entry->next;
        }
    }
    if (already_cached) {
        // nothing needs to be relocated.
    } else {
        // Create a new entry.
        new_entry->uuid = id.uuid;
        new_entry->type = id.type;
        // and, store the path on the heap and give it to the new resource entry.
        new_entry->path = (char *) malloc((strlen(resource_handle->data.path) + 1) * sizeof(char));
        mem_check(new_entry->path);
        strcpy(new_entry->path, resource_handle->data.path);
        // Next, load the resource in whatever way this resource type loads, by calling the load function in its resource-type-info table entry.
        void *resource = g_resource_type_info[new_entry->type].load(new_entry->path);
        if (resource == NULL) {
            fprintf(stderr, ERROR_ALERT "Failed to load resource from path \"%s\".\n", new_entry->path);
            exit(EXIT_FAILURE);
        }
        new_entry->resource = resource;
    }

    // Update the resource handle with the new id for the newly loaded resource. Now, it is valid for short-term usage by the caller.
    resource_handle->_id = id;
    if (DEBUG) printf("Resource loaded!\n");
    return resource_handle;
#undef DEBUG
}

bool reload_resource(ResourceHandle *handle)
{
    // Load the resource again into a separate place (not associated to the resource table).
    if (!handle->path_backed) {
        fprintf(stderr, ERROR_ALERT "Attempted to reload a non-path-backed resource.\n");
        exit(EXIT_FAILURE);
    }
    void *reloaded = g_resource_type_info[handle->_id.type].load(handle->data.path);
    // If it failed to reload, allow the caller to handle this. Don't do anything.
    if (reloaded == NULL) {
        //------error logging
        printf(ERROR_ALERT "Failed to reload resource with path \"%s\".\n", handle->data.path);
        //----------Edit the resource tree!!!!
        return false;
    }
    // If the resource type has an unload function, do this first. It will tear down the resource.
    void (*unload)(void *) = g_resource_type_info[handle->_id.type].unload;
    if (unload != NULL) {
        unload(___resource_data(handle));
    }
    // Free the base resource structure. The resource should not be null, but checking anyway.
    if (g_resource_table[handle->_id.table_index].resource != NULL) free(g_resource_table[handle->_id.table_index].resource);
    // Now, replace the old base structure with the new base structure.
    g_resource_table[handle->_id.table_index].resource = reloaded;
    return true;
}

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

static ResourceID null_resource_id(void)
{
    ResourceID id;
    id.uuid = 0;
    id.type = 0;
    id.table_index = 0;
    return id;
}


//--------------------------------------------------------------------------------
// Testing
//--------------------------------------------------------------------------------
void print_resource_types(void)
{
    printf("Resource types:\n");
    for (int i = 0; i < g_num_resource_types; i++) {
        printf("name: %.80s\n", g_resource_type_info[i].name);
        printf("type id: %d\n", g_resource_type_info[i].type);
        printf("size: %ld\n", g_resource_type_info[i].size);
        printf("has load function?: %s\n", g_resource_type_info[i].load == NULL ? "No" : "Yes");
    }
}

void print_resource_path(void)
{
    printf("RESOURCE PATH: ");
    if (g_resource_path == NULL) {
        printf("Not initialized\n");
    } else {
        printf("\"%s\"\n", g_resource_path);
    }
}
