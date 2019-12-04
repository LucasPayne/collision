/*--------------------------------------------------------------------------------
    Resources, resource management, and resource loading module.
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
} ResourceID;

typedef struct ResourceHandle_s {
    ResourceID id;
    char *path;
} ResourceHandle;

typedef struct ResourceTableEntry_s {
    ResourceUUID uuid;
    ResourceType type;
    void *resource;
} ResourceTableEntry;

#define RESOURCE_TABLE_START_SIZE 1024
static bool g_initialized_resource_table;
static Resource **g_resource_table;
static ResourceUUID g_last_resource_uuid = 0;


static ResourceID create_resource_id(ResourceType type)
{
    if (!g_initialized_resource_table) {
        g_resource_table_size = RESOURCE_TABLE_START_SIZE;
        g_resource_table = (Resource **) calloc(1, sizeof(Resource *) * g_resource_table_size);
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
                return new_id;
            }
        }
        uint32_t previous_size = g_resource_table_size;
        g_resource_table_size += RESOURCE_TABLE_START_SIZE; // grow the resource table linearly.
        g_resource_table = (Resource **) realloc(g_resource_table, sizeof(Resource *) * g_resource_table_size);
        mem_check(g_resource_table);
        // Null initialize the new available resource pointers.
        memset(g_resource_table + previous_size, 0, sizeof(Resource *) * (g_resource_table_size - previous_size));
    }
}
static void *get_resource(ResourceID id)
{
    if (g_resource_table[id.table_index] != NULL) {
        if (g_resource_table[id.table_index].uuid != id.uuid) return NULL;
        if (g_resource_table[id.table_index].type != id.type) return NULL;
        return g_resource_table[id.table_index].resource;
    }
}

// "create" resource. If this path is already loaded, returns the ID of the resource already in memory.
// Things that use resources get the resource IDs this way and don't know whether or not they caused a load or are sharing a resource loaded
// by another thing. Then, the thing should be able to consistently use the same ID _until_ it doesn't work. This doesn't mean it fails, just
// that it needs to create the resource again, meaning either it triggers a load or finds the resource path already loaded. Then, the resource ID
// must be updated. Anything else using the resource ID propogated from the first thing will trigger the same (since the uuid won't match), and since
// the first thing probably triggered a resource load, the things using the same resource are just going to recheck the path and update their ID copies.
static ResourceID create_resource(ResourceType type, char *path)
{
    ResourceID id = create_resource_id(type);
    g_resource_table[id.map_index].resource = calloc(1, g_resource_type_info[type].size);
    mem_check(g_resource_table[id.map_index]);
}

#define MAX_SHADER_NAME_LENGTH 32;
typedef struct Shader_s {
    ShaderID id;
    char *hash_path;
    char *resource_path;
    GLuint vram_id;
} Shader;


