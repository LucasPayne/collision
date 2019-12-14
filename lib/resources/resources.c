/*--------------------------------------------------------------------------------
    Resources, resource management, and resource loading module.
--------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "helper_definitions.h"
#include "resources.h"
//--------------------------------------------------------------------------------
// Static function declarations
//--------------------------------------------------------------------------------
static ResourceID create_resource_id(ResourceType type);
static ResourceHandle *relocate_resource_handle(ResourceHandle *resource_handle);
static ResourceTree *_add_or_get_resource_path(ResourceID id, char *path, bool adding);
static ResourceTree *add_resource_branch(ResourceTree *tree, char *name);
static ResourceTree *add_resource_leaf(ResourceTree *tree, char *name);
static ResourceTree *get_resource_tree_entry(ResourceTree *tree, char *name, bool is_leaf);
static ResourceTree *get_resource_leaf(ResourceTree *tree, char *name);
static ResourceTree *get_resource_branch(ResourceTree *tree, char *name);
static ResourceID null_resource_id(void);

// Helper functions and printing
static void _print_resource_tree(ResourceTree *tree, int indent);
static void make_indent(int indent);
//--------------------------------------------------------------------------------
// Resource types
static int g_num_resource_types = 0;
ResourceTypeInfo *g_resource_type_info = NULL;

// Resource table and ID allocation
static bool g_initialized_resource_table;
static uint32_t g_resource_table_size = 0;
ResourceTableEntry *g_resource_table;
static ResourceUUID g_last_resource_uuid = 0;

// Resource paths and search
static uint32_t g_resource_path_length = 0;
static char *g_resource_path = NULL;
static ResourceTree *g_resource_tree = NULL;

/*
Resource types are built up at runtime. This is so an application can include a call to a function for modules
which have different resource types to initialize these. Arbitrary resource types can then be introduced even at runtime. 
The resource type IDs are sorted out automatically, although they are not neccessarily consistent.

This function is non-static because a macro expands to it. Then
     add_resource_type(Texture)
expands to type metadata and passes in a global ResourceType pointer to Texture_RTID, gives it a size, the name of the structure,
and load function Texture_load. This allocates it an ID so further usage of macros which use the symbol Texture use this ID.
 */
void ___add_resource_type(ResourceType *type_pointer, size_t size, char *name, void *(*load)(char *))
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
    strncpy(new_info->name, name, MAX_RESOURCE_TYPE_NAME_LENGTH);
}
/*
This gives a new resource ID which should be used instantly after creation. When returned, this is a usable ID, so
other functions (probably just the one that uses this) can create the new resource entry at the given table index
and with the allocated uuid.
*/
static ResourceID create_resource_id(ResourceType type)
{
    printf("creating resource id ...\n");
    if (!g_initialized_resource_table) {
        g_resource_table_size = RESOURCE_TABLE_START_SIZE;
        g_resource_table = (ResourceTableEntry *) calloc(1, sizeof(ResourceTableEntry) * g_resource_table_size);
        mem_check(g_resource_table);
        g_initialized_resource_table = true;
        printf("initialized resource table.\n");
    }
    int i = 0;
    while (1) {
        for (; i < g_resource_table_size; i++) {
            if (g_resource_table[i].uuid == 0) {
                ResourceID new_id;
                new_id.table_index = i;
                new_id.uuid = ++g_last_resource_uuid;
                new_id.type = type;
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
    if (resource_handle->_path != NULL) free(resource_handle->_path);
    resource_handle->_id = null_resource_id();
    resource_handle->_id.type = resource_type;
    resource_handle->_path = (char *) malloc((strlen(path) + 1) * sizeof(char));
    mem_check(resource_handle->_path);
    strcpy(resource_handle->_path, path);
}
ResourceHandle ___new_resource_handle(ResourceType resource_type, char *path)
{
    ResourceHandle resource_handle;
    resource_handle._id = null_resource_id();
    resource_handle._id.type = resource_type;
    resource_handle._path = (char *) malloc((strlen(path) + 1) * sizeof(char));
    mem_check(resource_handle._path);
    strcpy(resource_handle._path, path);
    return resource_handle;
}

/*
The caller should not know or care whether a dereference caused a relocation.
The dereference should be safe for the function using it but shouldn't be saved. The pointer is returned back only for
the convenience of relocating/validating and using an attribute in the same expression.
*/
static ResourceHandle *relocate_resource_handle(ResourceHandle *resource_handle)
{
#define DEBUG 0
    // If the resource handle's id is null, then a lookup needs to be done first instead of trying the table.
    if (resource_handle->_id.uuid != 0) {
        // Table lookup the resource. The null uuid is 0, so an empty entry cannot be matched.
        if (g_resource_table[resource_handle->_id.table_index].uuid == resource_handle->_id.uuid) {
            // The point of this. Resource loading and unloading should be very rare compared to references to the resource,
            // so that should be a constant fast lookup, yet still trigger a resource load if needed, unknown to the caller.
            if (DEBUG) printf("Handle dereferenced straight to the resource table.\n");
            return resource_handle;
        }
    }
    // Try walk the resource tree to see if this path is associated to an active resource ID.
    if (resource_handle->_path == NULL) {
        fprintf(stderr, ERROR_ALERT "Attempted to dereference resource handle which has no resource path.\n");
        exit(EXIT_FAILURE);
    }
    ResourceID id = lookup_resource(resource_handle->_path);
    if (id.uuid != 0) {
        // Resource is loaded. Update the handle with the ID found from querying.
        resource_handle->_id = id;
        if (DEBUG) printf("Resource found by a lookup!\n");
        return resource_handle;
    }
    // The resource isn't loaded. Trigger a load.
    // First, create a new entry in the resource table with a new ID.
    id = create_resource_id(resource_handle->_id.type);
    ResourceTableEntry *new_entry = &g_resource_table[id.table_index];
    new_entry->uuid = id.uuid;
    new_entry->type = id.type;
    // and, store the path on the heap and give it to the new resource entry.
    new_entry->path = (char *) malloc((strlen(resource_handle->_path) + 1) * sizeof(char));
    mem_check(new_entry->path);
    strcpy(new_entry->path, resource_handle->_path);
    // Next, load the resource in whatever way this resource type loads, by calling the load function in its resource-type-info table entry.
    void *resource = g_resource_type_info[new_entry->type].load(new_entry->path);
    if (resource == NULL) {
        fprintf(stderr, ERROR_ALERT "Failed to load resource from path \"%s\".\n", new_entry->path);
        exit(EXIT_FAILURE);
    }
    new_entry->resource = resource;
    // Finally, add this path to the resource tree, filling the tree enough to store the id at the leaf, for querying.
    add_resource(id, resource_handle->_path);

    // Update the resource handle with the new id for the newly loaded resource. Now, it is valid for short-term usage by the caller.
    resource_handle->_id = id;
    if (DEBUG) printf("Resource loaded!\n");
    return resource_handle;
#undef DEBUG
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
    // Initialize the resource tree if it is not initialized.
    if (g_resource_tree == NULL) {
        g_resource_tree = (ResourceTree *) calloc(1, sizeof(ResourceTree));
        mem_check(g_resource_tree);
    }

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
    if (!resource_file_path(path, suffix, path_buffer, 1024)) {
        return NULL;
    }
    //@@@
    /* printf("MATCHED PATH: \"%s\"\n", path_buffer); */
    FILE *file = fopen(path_buffer, flags);
    return file;
}

bool resource_file_path(char *path, char *suffix, char *path_buffer, int path_buffer_size)
{
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

        for (int i = 0; i < drive_length; i++) putchar(drive[i]);
        if (strlen(drive_buffer) == drive_length && strncmp(drive_buffer, drive, drive_length) == 0) {
            // Checking lengths since a strncmp can have equal prefixes yet the drive buffer stores a longer drive name.
            return true;
        }

        prefix = strchr(prefix, ':');
        prefix = strchr(prefix + 1, ':');
        if (prefix != NULL) prefix ++;

    } while (prefix != NULL);
    return false;
buffer_size_error:
    fprintf(stderr, ERROR_ALERT "Resource path too large for given buffer.\n");
    exit(EXIT_FAILURE);
}

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
static ResourceTree *get_resource_branch(ResourceTree *tree, char *name)
{
    return get_resource_tree_entry(tree, name, false);
}
static ResourceTree *get_resource_leaf(ResourceTree *tree, char *name)
{
    return get_resource_tree_entry(tree, name, true);
}

static ResourceTree *_add_or_get_resource_path(ResourceID id, char *path, bool adding)
{
    /*
    Adding:
         Fills out the resource tree enough to store the id at the leaf of this path branch. Returns
         the resource ID of the loaded resource. Adding should only be done when a resource is loaded.
    Not adding:
         Attempts to traverse the resource tree to see if a path corresponds to a loaded resource. If it isn't return NULL,
         if it is, return the resource ID at the leaf (the resource ID of the loaded resource).
    */
    
    if (g_resource_tree == NULL) {
        g_resource_tree = (ResourceTree *) calloc(1, sizeof(ResourceTree));
        mem_check(g_resource_tree);
    }

    // Start at the resource tree root.
    ResourceTree *tree = g_resource_tree;
    while (1) {
        if (path[0] == '\0') {
            fprintf(stderr, ERROR_ALERT "Attempted to add a resource path with an empty entry name.");
            exit(EXIT_FAILURE);
        }
        char *end = strchr(path, '/');
        int entry_length = end - path;
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
        char entry_buffer[128];
        int i;
        for (i = 0; i < entry_length; i++) {
            entry_buffer[i] = path[i];
        }
        entry_buffer[i] = '\0';
        if (adding) {
            // Finds or creates a branch so the addition can continue. (?)
            tree = add_resource_branch(tree, entry_buffer);
        } else {
            // Tries to find a branch. If not found, return NULL because the queried-for resource is not in the resource tree.
            tree = get_resource_branch(tree, entry_buffer);
            if (tree == NULL) return NULL;
        }
        path = end + 1;
    }
    fprintf(stderr, ERROR_ALERT "Attempted to add resource path with invalid drive.\n");
    exit(EXIT_FAILURE);
}

ResourceID add_resource(ResourceID id, char *path)
{
    ResourceTree *added_resource_tree = _add_or_get_resource_path(id, path, true);
    if (added_resource_tree == NULL) {
        fprintf(stderr, ERROR_ALERT "Could not add resource to resource tree.\n");
        exit(EXIT_FAILURE);
    }
    if (!added_resource_tree->is_leaf) {
        fprintf(stderr, ERROR_ALERT "Something went wrong. Return from adding a resource got a resource directory, not a leaf resource.\n");
        exit(EXIT_FAILURE);
    }
    //--------- ???
    if (added_resource_tree->contents.resource_id.uuid != 0) {
        return added_resource_tree->contents.resource_id;
    }
    added_resource_tree->contents.resource_id = id;
    return id;
}

ResourceID lookup_resource(char *path)
{
    ResourceTree *got_resource_tree = _add_or_get_resource_path(null_resource_id(), path, false);
    if (got_resource_tree == NULL) {
        return null_resource_id();
    }
    if (!got_resource_tree->is_leaf) {
        fprintf(stderr, ERROR_ALERT "Something went wrong. Return from a resource lookup got a resource directory, not a leaf resource.\n");
        exit(EXIT_FAILURE);
    }
    return got_resource_tree->contents.resource_id;
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
static void test_lookup_resource(char *path)
{
    ResourceID id = lookup_resource(path);
    if (id.uuid == 0) {
        printf("\"%s\" could not be found.\n", path);
    } else {
        printf("\"%s\" was found!\n", path);
        printf("uuid: %ld\n", id.uuid);
    }
}
void test_resource_tree(void)
{
    resource_path_add("TextureLibrary", "/path/to/texture/library");
    resource_path_add("Project", "/path/to/project/stuff");
    resource_path_add("Application", "/another/path/to/application/specific/stuff");

    printf("Resource path:\n\t%s\n", g_resource_path);

    ResourceID id;
    id.uuid = 650;
    add_resource(id, "Project/dolphin");
    id.uuid = 400;
    add_resource(id, "Project/thing");
    id.uuid = 1337;
    add_resource(id, "Project/stuff");
    id.uuid = 30;
    add_resource(id, "TextureLibrary/stuffo");
    id.uuid = 1;
    add_resource(id, "Application/stuff/path/to/the/stuff");
    id.uuid = 1000;
    add_resource(id, "TextureLibrary/dolphin");
    id.uuid = 100;
    add_resource(id, "TextureLibrary/bungus/dolphin");
    id.uuid = 333;
    add_resource(id, "Project/bungus/path/to/the");

    print_resource_tree();

    test_lookup_resource("choochoo");
    test_lookup_resource("choochoo");
    test_lookup_resource("choochoo/chooooo");
    test_lookup_resource("choochoo/chooooseeeyouuu");
    test_lookup_resource("Project/dolphin");
    test_lookup_resource("Project/dolphine");
    test_lookup_resource("Application/stuff/path/to/the/stuff");
    test_lookup_resource("Project/bungus/path/to/thee");
    test_lookup_resource("Project/bungus/path/to/the");
    test_lookup_resource("Project/bungus/path/to/t");
}

void print_resource_tree(void)
{
    printf("Resource tree:\n");
    _print_resource_tree(g_resource_tree, 0);
}

static void make_indent(int indent)
{
    for (int i = 0; i < indent; i++) printf("  ");
}

static void _print_resource_tree(ResourceTree *tree, int indent)
{
    if (tree->is_leaf) {
        make_indent(indent); printf("Leaf:\n");
        make_indent(indent + 1); printf("name: %.80s\n", tree->name);
        make_indent(indent + 1); printf("uuid: %ld\n", tree->contents.resource_id.uuid);
        return;
    }

    make_indent(indent); printf("Branch:\n");
    make_indent(indent + 1); printf("name: %.80s\n", tree->name);
    if (tree->contents.first_child != NULL) _print_resource_tree(tree->contents.first_child, indent + 1);

    ResourceTree *next = tree->next;
    while (next != NULL) {
        _print_resource_tree(next, indent);
        next = next->next;
    }
}

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

