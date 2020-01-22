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

void init_resource_system(void)
{
    static const uint8_t sma_pool_powers = { // Edit this to change the available pool sizes.
        3, //8
        4, //16
        5, //32
        6, //64
        7, //128
    };
    static const int num_sma_pool_powers = sizeof(sma_pool_powers)/sizeof(uint8_t);
    init_small_memory_allocator(sma_pool_powers, num_sma_pool_powers);
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
