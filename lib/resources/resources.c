/*--------------------------------------------------------------------------------
    Resources, resource management, and resource loading module.


notes:
    ------- Get rid of "drives" (?)
There could be a problem with paths on the heap. How do they deallocate? What references
paths?

References don't start at an ID, they do a path lookup and trigger a load if it isn't there. This gets an ID.
Usage of the ID must always be preceded by a potential relocation.

Resource handles. "Dereferencing" a resource handle like a C++ * prefix operator override.
This does:
    table index lookup of resource
    path lookup of resource
    resource load

May do: don't load at start, just set the id to null. Then, have the resource handle dereference handler
treat that as just a lookup miss, and try to find the loaded resource or trigger a load.
This may make it harder to detect faulty resources, but tests could be written for that and I don't think that'll be a problem.

--------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "resources.h"
//--------------------------------------------------------------------------------
// Static function declarations
//--------------------------------------------------------------------------------
static ResourceID create_resource_id(ResourceType type, char *path);
static ResourceID create_resource(ResourceType type, char *path);
static ResourceID relocate_resource_handle(ResourceHandle *resource_handle);
static ResourceTree *_add_or_get_resource_path(ResourceID id, char *path, bool adding);
static ResourceTree *add_resource_branch(ResourceTree *tree, char *name);
static ResourceTree *add_resource_leaf(ResourceTree *tree, char *name);
static ResourceTree *get_resource_tree_entry(ResourceTree *tree, char *name, bool is_leaf);
//--------------------------------------------------------------------------------

static int g_num_resource_types = 0;
static ResourceTypeInfo *g_resource_type_info = NULL;

void ___add_resource_type(ResourceType *type_pointer, size_t size, char *name)
{
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
    strncpy(new_info->name, name, MAX_RESOURCE_TYPE_INFO_NAME_LENGTH);
}
void *___resource_data(ResourceType type, MeshHandle handle)
{
    if (type != handle._id.type) {
        fprintf(stderr, ERROR_ALERT "Type error when dereferencing resource handle.\n");
        exit(EXIT_FAILURE);
    }
    return g_resource_table[relocate_resource_handle(&( HANDLE ))._id.map_index].resource;
}
void ___load_resource(ResourceType type, ResourceHandle handle)
{
    g_resource_type_info[type].load(&handle);
}
// Usage:
//      Shader *vertex_shader = resource_data(Shader, renderer->shaders[Vertex]);

static ResourceID relocate_resource_handle(ResourceHandle *resource_handle)
{
    /* Returns resource ID in handle after relocation/staying put. The caller should not know or care whether a dereference caused a relocation. */
    // Table lookup the resource.
    //================================================================================
    if (g_resource_table[resource_handle->id.table_index].uuid == resource_handle->id.uuid) {
        // The point of this. Resource loading and unloading should be very rare compared to references to the resource,
        // so that should be a constant fast lookup, yet still trigger a resource load if needed, unknown to the caller.
        return id;
    }
    //================================================================================
    // If the table lookup didn't work, the resource may still be loaded somewhere else. This either gets that through
    // the path lookup, or if that fails, loads the resource and returns the ID it is given.
    ResourceID found_id = create_resource(resource_handle->id.type, resource_handle->_path);
    // Update the resource handle.
    resource_handle->_id = found_id;
    return found_id;
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
    /* char *physical_filepath */
    /* g_resource_type_info.load_resource( */
    /* free(name); */
    //================================================================================
    
    return id;
}

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


static ResourceTree *get_resource_tree_entry(ResourceTree *tree, char *name, bool is_leaf)
{
    /* Returns NULL if the entry is not found in the tree's flat directory. 
     * Resource directory structure is recursive, but this function is not. It should be used with a recursive function e.g. for finding resources.
     */
    if (tree->is_leaf) {
        fprintf(stderr, ERROR_ALERT "Attempted to treat a resource leaf as a resource fork.\n");
        exit(EXIT_FAILURE);
    }
    if (strlen(name) > MAX_RESOURCE_BRANCH_NAME_LENGTH) {
        fprintf(stderr, ERROR_ALERT "Given name length is too long. Maximum is set to %d.\n", MAX_RESOURCE_BRANCH_NAME_LENGTH);
        exit(EXIT_FAILURE);
    }
    ResourceTree *cur_subtree = tree->contents.first_child;
    if (cur_subtree != NULL) {
        do {
            if (strcmp(name, cur_subtree->name) == 0) {
                if (cur_subtree->is_leaf != is_leaf) {
                    // This name at this flat level is for an entry of the wrong kind.
                    return NULL;
                }
                // The resource tree entry was found in this flat directory.
                return cur_subtree;
            }
            cur_subtree = cur_subtree->next;
        } while (cur_subtree != NULL);
    }
    // It wasn't found.
    return NULL;
}

static ResourceTree *add_resource_tree_entry(ResourceTree *tree, char *name, bool is_leaf)
{

    ResourceTree *got_entry = get_resource_tree_entry(tree, name, is_leaf);
    if (got_entry != NULL) {
        // Found it, don't need to create it.
        return got_entry;
    }
    // This resource entry doesn't exist in this flat directory, create it.
    ResourceTree *new_subtree = (ResourceTree *) calloc(1, sizeof(ResourceTree));
    mem_check(new_subtree);
    strncpy(new_subtree->name, name, MAX_RESOURCE_BRANCH_NAME_LENGTH);
    new_subtree->is_leaf = is_leaf;

    if (tree->contents.first_child == NULL) {
        // Directory is empty, add this as the first child.
        tree->contents.first_child = new_subtree;
        return new_subtree;
    }
    // Insert at the beginning of the directory's entry linked list.
    new_subtree->next = tree->contents.first_child;
    tree->contents.first_child = new_subtree;

    return new_subtree;
}
//---- inlining?
static ResourceTree *add_resource_branch(ResourceTree *tree, char *name)
{
    return add_resource_tree_entry(tree, name, false);
}
static ResourceTree *add_resource_leaf(ResourceTree *tree, char *name)
{
    return add_resource_tree_entry(tree, name, true);
}

static ResourceTree *_add_or_get_resource_path(ResourceID id, char *path, bool adding)
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
            path = strchr(path, '\0') + 1; // go past the drive name.
            while (1) {
                if (path == '\0') {
                    fprintf(stderr, ERROR_ALERT "Attempted to add a resource path with an empty entry name.");
                    exit(EXIT_FAILURE);
                }
                char *end = strchr(path, '/');
                if (end == NULL) {
                    // this is a leaf.
                    if (adding) {
                        ResourceTree *resource_id_leaf = add_resource_leaf(tree, path);
                        resource_id_leaf->contents.resource_id = id;
                        return resource_id_leaf;
                    } else {
                        ResourceTree *resource_id_leaf = get_resource_leaf(tree, path);
                        return resource_id_leaf; // NULL if it wasn't found.
                    }
                }
                end[0] = '\0';
                if (adding) {
                    // Finds or creates a branch so the addition can continue. (?)
                    tree = add_resource_branch(tree, path);
                } else {
                    // Tries to find a branch. If not found, return NULL because the queried-for resource is not in the resource tree.
                    tree = get_resource_branch(tree, path);
                    if (tree == NULL) return NULL;
                }
                path = end + 1;
            }
        }
    }
    fprintf(stderr, ERROR_ALERT "Attempted to add resource path with invalid drive.\n");
    exit(EXIT_FAILURE);
}

ResourceID add_resource(ResourceID id, char *path)
{
    ResourceTree *added_resource_tree = _add_or_get_resource_path(id, path, true);
    if (!got_resource_tree->is_leaf) {
        fprintf(stderr, ERROR_ALERT "Something went wrong. Return from adding a resource got a resource directory, not a leaf resource.\n");
        exit(EXIT_FAILURE);
    }
    return added_resource_tree->contents.resource_id;
}

ResourceID lookup_resource(ResourceID id, char *path)
{
    ResourceTree *got_resource_tree = _add_or_get_resource_path(id, path, false);
    if (got_resource_tree == NULL) {
        return NULL_RESOURCE_ID;
    }
    if (!got_resource_tree->is_leaf) {
        fprintf(stderr, ERROR_ALERT "Something went wrong. Return from a resource lookup got a resource directory, not a leaf resource.\n");
        exit(EXIT_FAILURE);
    }
    return got_resource_tree->contents.resource_id;
}
