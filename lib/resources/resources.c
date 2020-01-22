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
// This dictionary is the "root", and each resource has a GUID, naturally its path from the root, with no /-prefix.
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
    id.table_index = 0;
    return id;
}

/*================================================================================
    Resource system and memory initialization.
================================================================================*/
void init_resource_system(void)
{
    // Initialize the small memory allocator, where the resource data will be allocated.
    // Could initialize the small memory allocator elsewhere if, for example, the entity system would want to use it.
    static const uint8_t sma_pool_powers = { // Edit this to change the available pool sizes.
        3, //8
        4, //16
        5, //32
        6, //64
        7, //128
    };
    static const int num_sma_pool_powers = sizeof(sma_pool_powers)/sizeof(uint8_t);
    init_small_memory_allocator(sma_pool_powers, num_sma_pool_powers);

    // Initialize the resource cache.
}

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
    handle->_id.uuid = hash_crc32(handle->data.path); // hopefully universally unique.
    resource_handle.data.path = (char *) malloc((strlen(path) + 1) * sizeof(char));
    mem_check(resource_handle.data.path);
    strcpy(resource_handle.data.path, path);
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

    ResourceTableEntry *checking_entry = &g_resource_table[resource_handle->_id.uuid % RESOURCE_TABLE_SIZE];
    ResourceTableEntry *new_entry = checking_entry; // At the end of search, if the resource wasn't found cached, this will be left as a pointer to fill with the new loaded entry.
                                                    // This complication is here since a new entry can be added either straight in the table, or at the end of one of the chains.
    if (checking_entry->uuid != 0) {
        while (1) {
            if (checking_entry->uuid == resource_handle->_id.uuid) {
                // The point of this. Resource loading and unloading should be very rare compared to references to the resource,
                // so that should be a constant (-except chaining) fast lookup, yet still trigger a resource load if needed, unknown to the caller.
                return checking_entry->resource;
            }
            if (checking_entry->next == NULL) {
                // The end of the chain was reached, and no cached resource was found. Allocate a new entry and attach it to the end of the chain.
                checking_entry->next = (ResourceTableEntry *) sma_alloc(sizeof(ResourceTableEntry)); // Using the small memory allocator here as well, to store the chains of the hash table.
                new_entry = checking_entry->next;
                break;
            }
            checking_entry = checking_entry->next;
        }
    }
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

