/*--------------------------------------------------------------------------------
    Resources, resource management, and resource loading module.

notes:
There could be a problem with paths on the heap. How do they deallocate? What references
paths?

References don't start at an ID, they do a path lookup and trigger a load if it isn't there. This gets an ID.
Usage of the ID must always be preceded by a potential relocation.

--------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>


typedef uint64_t ResourceUUID;
typedef uint32_t ResourceType;

typedef struct ResourceID_s {
    uint32_t table_index;
    ResourceUUID uuid;
    ResourceType type;
    char *path;
} ResourceID;


typedef struct ResourceTableEntry_s {
    ResourceUUID uuid;
    ResourceType type;
    char *path;
    void *resource;
} ResourceTableEntry;

#define RESOURCE_TABLE_START_SIZE 1024
static bool g_initialized_resource_table;
static ResourceTableEntry *g_resource_table;
static ResourceUUID g_last_resource_uuid = 0;


static ResourceID create_resource_id(ResourceType type, char *path)
{
    if (!g_initialized_resource_table) {
        g_resource_table_size = RESOURCE_TABLE_START_SIZE;
        g_resource_table = (ResourceTableEntry *) calloc(1, sizeof(ResourceTableEntry) * g_resource_table_size);
        mem_check(g_resource_table);
        g_initialized_resource_table = true;
    }
    int i = 0;
    while (1) {
        for (; i < g_resource_table_size; i++) {
            if (g_shader_table[i] == NULL) {
                ResourceID new_id;
                new_id.table_index = i;
                new_id.uuid = ++g_last_resource_uuid;
                new_id.type = type;
                new_id.path = path;
                return new_id;
            }
        }
        uint32_t previous_size = g_resource_table_size;
        g_resource_table_size += RESOURCE_TABLE_START_SIZE; // grow the resource table linearly.
        g_resource_table = (ResourceTableEntry *) realloc(g_resource_table, sizeof(ResourceTableEntry) * g_resource_table_size);
        mem_check(g_resource_table);
        // Null initialize the new available resource table entries.
        memset(g_resource_table + previous_size, 0, sizeof(ResourceTableEntry) * (g_resource_table_size - previous_size));
    }
}

/* static void *get_resource(ResourceID id) */
/* { */
/*     if (g_resource_table[id.table_index] != NULL) { */
/*         if (g_resource_table[id.table_index].uuid != id.uuid) return NULL; */
/*         if (g_resource_table[id.table_index].type != id.type) return NULL; */
/*         return g_resource_table[id.table_index].resource; */
/*     } */
/* } */

typedef struct ResourceTreeFork_s {
    struct ResourceTree_s *next;
    struct ResourceTree_s *first_child;
} ResourceTreeFork;
typedef struct ResourceTree_s {
    bool is_leaf;
    union contents {
        ResourceTreeFork fork;
        ResourceID resource_id;
    };
} ResourceTree;

static void add_resource_path(ResourceID id, char *path)
{
    /* Fills out the resource forest enough to store the id at the leaf of this path branch. */
    char *drive_end = strchr(path, '/');
    if (drive_end == NULL) {
        fprintf(stderr, ERROR_ALERT "Invalid resource path \"%s\".\n", path);
        exit(EXIT_FAILURE);
    }
    drive_end[0] = '\0';
    for (int i = 0; i < g_num_resource_drives; i++) {
        if (strcmp(path, g_resource_drive_info[i].name) == 0) {
            // Put this resource path into this resource drive's tree.
            ResourceTree *tree = &g_resource_drive_info[i].tree;
        }
    }
    fprintf(stderr, ERROR_ALERT "Attempted to add resource path with invalid drive, \"%s\".\n", path);
    exit(EXIT_FAILURE);
}

static ResourceID find_resource(ResourceType type, char *path)
{
    /* Parse the path to find a resource if it is loaded. If it is not loaded,
     * returns NULL.
     */
    // Lookup resource
    //================================================================================

    char *p = path;
    while (1) {
        char *end = strchr(p, '/');
        if (end == NULL) break;

        

        p = end + 1;
    }
    // resource name


    //================================================================================
    return NULL_RESOURCE_ID;
}

// "create" resource. If this path is already loaded, returns the ID of the resource already in memory.
// Things that use resources get the resource IDs this way and don't know whether or not they caused a load or are sharing a resource loaded
// by another thing. Then, the thing should be able to consistently use the same ID _until_ it doesn't work. This doesn't mean it fails, just
// that it needs to create the resource again, meaning either it triggers a load or finds the resource path already loaded. Then, the resource ID
// must be updated. Anything else using the resource ID propogated from the first thing will trigger the same (since the uuid won't match), and since
// the first thing probably triggered a resource load, the things using the same resource are just going to recheck the path and update their ID copies.
static ResourceID create_resource(ResourceType type, char *path)
{
    ResourceID id = find_resource(type, path);
    if (id != NULL_RESOURCE_ID) {
        // Resource is loaded. Return the resource ID, but caller doesn't know it isn't "creating" it.
        return id;
    }
    // Load the resource
    //================================================================================
    // This resource isn't loaded. Actually create the resource.
    id = create_resource_id(type, path);
    g_resource_table[id.table_index].uuid = id.uuid;
    g_resource_table[id.table_index].type = type;
    // Allocate and zero-initialize the memory needed for this type of resource, and give to the new resource entry.
    g_resource_table[id.table_index].resource = calloc(1, g_resource_type_info[type].size);
    mem_check(g_resource_table[id.table_index].resource);
    // Store the path on the heap and give to the new resource entry.
    g_resource_table[id.table_index].path = (char *) malloc((strlen(path) + 1) * sizeof(char));
    mem_check(g_resource_table[id.table_index].path);
    strcpy(g_resource_table[id.table_index].path, path);

    // ...
    //================================================================================
    
    return id;
}

// Similar to realloc, the use of this function must reset the identifier, since it may be moved.
// void *resource;
// self->thing = get_resource(&resource, self->thing);
static ResourceID get_resource(void **resource, ResourceID id)
{
    // Table lookup the resource.
    //================================================================================
    if (g_resource_table[id.table_index].uuid == id.uuid) {
        // The point of this. Resource loading and unloading should be very rare compared to references to the resource,
        // so that should be a constant fast lookup, yet still trigger a resource load if needed, unknown to the caller.
        *resource = g_resource_table[id.table_index].resource;
        return id;
    }
    //================================================================================
    // If the table lookup didn't work, the resource may still be loaded somewhere else. This either gets that through
    // the path lookup, or if that fails, loads the resource.
    return create_resource(id.type, id.path);
}

