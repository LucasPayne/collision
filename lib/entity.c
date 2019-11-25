/*-------------------------------------------------------------------------------- 
   Definitions for the entity model.
   See the header for details.

    Technical problem?
    Object reference. There could be two concepts of IDs: one that is a consistent map lookup ID to
    get a pointer to the object, and one which is unique over all objects that may have been destroyed.
    These are universally unique independent of any object's lifetime. This can be done by incrementing
    an int/long and trusting that it would be highly unlikely to run into problems. This seems inconvenient for a map
    lookup structure, yet avoids the problem of something holding an object reference not being sure whether
    the object is what it thinks (if it is destroyed, something holding a reference should know, and that
    reference should not be replaced by some other object. With structure casts etc. which I am using now this could
    be very bad).

    So, maybe mix the two IDs:
        Map lookup index:
            Consistent over an object's lifetime.
            Does not give trustworthy information on whether an object is destroyed, since this index can
            be reallocated.
        Universally-temporally unique ID:
            (with increment allocation, could be thought of as the "time" the object was created.)
            Consistent even after the object has been destroyed. This will not be used for map lookups.
            This gives information on whether a reference is valid: if the map lookup doesn't get a null pointer,
            the object's UTUID is checked. If it is different, then the object must have been destroyed and the
            index reallocated, so treat this logically the same as if there had been a null pointer at this
            index.

    Error handling: at what level? Caller?

    Off by one errors and string functions: check these.
--------------------------------------------------------------------------------
What about:
differentiate between "managers" and "environmental processes".
A manager reroutes calls to aspect bookkeeping functions. Creating IDs, initializing aspects.
It can also have control over its index map, as long as it doesn't shuffle the map indices around,
for example reordering aspect data in memory and changing the pointers in the map.

There is a standard interface to create aspects. This way, aspects of a thing are distributed for control
by different managers, yet are still "owned" by the entity ID. Environmental processes can be tightly associated
to a manager or be arbitrary processes that query/subscribe to different aspect types.
The point of using a manager (?) is to prepare data for specific types of processing (mesh handles, collision data,
images, GUI elements, debug wireframes, misc. object logic, ...). An environmental process may want to iterate
over data in a way that is not just going through and seeing what is not null in the aspect map. This may be an option,
but the aspect map could just be for random access, correct/safe constant time lookup of aspects, and traversal
of the entity-aspect model. For data processing, a manager may provide an iterator which itself may just be
the map traversal, or take advantage of contiguity/structure bookkept by the manager to iterate possibly directly
over a memory region.

What about: If an aspect is created and there is no manager for its type, then there is a default manager which just mallocs.
--------------------------------------------------------------------------------
Aspect types, structures and usage.
Could have:
Macro for having standard metadata for aspect data, aspect ID and entity ID.
This is inline (no substruct to access) but the pointer to the aspect data can
be cast to an AspectProperties so it is "inherited" from this.

----
num_aspects right now is actually the size of the space for aspects

managers: data guardians

probably better to keep initial sizes small and force the data bookkeeping to be done, for testing

--------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "helper_definitions.h"
#include "entity.h"

static bool entity_model_active = false;
#define ENTITY_MAP_START_SIZE 4
static EntityMapEntry *entity_map = NULL;
static unsigned int entity_map_size = 0;

static UUID last_uuid = 0;

#define START_NUM_MANAGERS 10
#define START_NUM_MANAGER_ASPECTS 2 // for now? same methods for creating maps
static Manager *managers = NULL;
static int managers_array_size = 0;


/*--------------------------------------------------------------------------------
enum AspectTypes {
    AspectType_Camera,
    AspectType_MeshRenderer,
    NUM_ASPECT_TYPES
};

#define ASPECT()\
    AspectID aspect_id;\
    EntityID entity_id;

typedef struct Camera_s {
ASPECT()
    Matrix4x4f projection_matrix;
    Renderer renderer;
} Camera;

typedef struct MeshRenderer_s {
ASPECT()
    MeshHandle mesh_handle;
} MeshRenderer;

static size_t aspect_type_sizes[NUM_ASPECT_TYPES] = {
    sizeof(Camera),
    sizeof(MeshRenderer)
};

EntityID entity = new_entity();
MeshRenderer *mesh_renderer = (MeshRenderer *) get_aspect_data(entity_add_aspect(entity, AspectType_MeshRenderer));

Could change name to _entity_add_aspect

// Straight to getting data from this ID
#define entity_add_aspect_get(ENTITY,ASPECT_NAME)\
    (ASPECT_NAME *) get_aspect_data(_entity_add_aspect(( ENTITY ), AspectType_ ## ASPECT_NAME))


EntityID entity = new_entity();
// static data? transform matrix, name, ..., stuff multiple aspects may use/control/read
Mesh mesh;
create_cube_mesh(&mesh);
MeshRenderer *mesh_renderer = entity_add_aspect(entity, MeshRenderer);
upload_mesh(&mesh_renderer->mesh_handle, &mesh);
Solid *solid = entity_add_aspect(entity, Solid);
copy_mesh(&mesh, &solid->collision_mesh);
solid->weight = 12.5;

--------------------------------------------------------------------------------*/
static void entity_extend_aspects(EntityID entity);
static void new_aspect(EntityID entity, AspectID aspect);
static AspectID create_aspect_id(AspectType type);
static void extend_aspect_map(Manager *manager);
static void *get_aspect_data(AspectID aspect);
static void extend_manager_array(void);
static AspectID *get_entity_aspects(EntityID entity);
static EntityID create_entity_id(void);
static void extend_entity_map(void);
static Manager *manager_of_type(AspectType type);


static size_t *aspect_type_sizes = NULL;


void init_entity_model(void)
{
    if (entity_model_active) {
        fprintf(stderr, "ERROR: entity model is already active.\n");
        exit(EXIT_FAILURE);
    }
    entity_map_size = ENTITY_MAP_START_SIZE;
    entity_map = (EntityMapEntry *) calloc(entity_map_size, sizeof(EntityMapEntry));
    mem_check(entity_map);


    managers_array_size = START_NUM_MANAGERS;
    managers = (Manager *) calloc(managers_array_size, sizeof(Manager));
    mem_check(managers);

    // for now, testing with some different aspect types
    const int testing_num_aspect_types = 5;
    aspect_type_sizes = (size_t *) calloc(testing_num_aspect_types, sizeof(size_t));
    for (int i = 0; i < testing_num_aspect_types; i++) {
        aspect_type_sizes[i] = 128;
    }
    mem_check(aspect_type_sizes);

    entity_model_active = true;
}

static void extend_entity_map(void)
{
    // Linearly increase the amount of space
    entity_map_size += ENTITY_MAP_START_SIZE;
    entity_map = (EntityMapEntry *) realloc(entity_map, sizeof(EntityMapEntry) * entity_map_size);
    mem_check(entity_map);
}

static EntityID create_entity_id(void)
{
    /* Creating a new entity id does not actually do anything with the map (except extend it possibly).
     * No objects are created and no map index is used up. However, a uuid is used,
     * but I think this is fine. This just gives an EntityID that can be used to create
     * an entity straight after calling.
     */
    EntityID id;
    id.uuid = ++last_uuid;
    while (1) {
        for (int i = 0; i < entity_map_size; i++) {
            if (entity_map[i].aspects == NULL) {
                id.map_index = i;
                return id;
            }
        }
        extend_entity_map();
    }
}

EntityID new_entity(int start_num_aspects)
{
    EntityID id = create_entity_id();
    entity_map[id.map_index].uuid = id.uuid;
    entity_map[id.map_index].num_aspects = start_num_aspects;
    entity_map[id.map_index].aspects = (AspectID *) malloc(sizeof(AspectID) * start_num_aspects);
    mem_check(entity_map[id.map_index].aspects);
    for (int i = 0; i < start_num_aspects; i++) {
        entity_map[id.map_index].aspects[i].map_index = 0;
        entity_map[id.map_index].aspects[i].uuid = 0;
        entity_map[id.map_index].aspects[i].type = 0; // "null" type
    }
    entity_map[id.map_index].static_data = NULL;

    return id;
}


static AspectID *get_entity_aspects(EntityID entity)
{
    // rather than a "get_entity", straight to the aspects list
    if (entity_map[entity.map_index].aspects == NULL) {
        return NULL;
    }
    if (entity_map[entity.map_index].uuid != entity.uuid) {
        return NULL;
    }
    return entity_map[entity.map_index].aspects;
}



Manager *new_manager(AspectType type, void (*new_aspect)(Manager *, AspectID), void (*destroy_aspect)(Manager *, AspectID), void (*aspect_iterator) (Iterator *))
{
    while (1) {
        for (int i = 0; i < managers_array_size; i++) {
            if (managers[i].type == 0) {
                managers[i].type = type;
                managers[i].new_aspect = new_aspect;
                managers[i].destroy_aspect = destroy_aspect;
                managers[i].aspect_iterator = aspect_iterator;
                managers[i].aspect_map_size = START_NUM_MANAGER_ASPECTS;
                managers[i].aspect_map = (void **) calloc(managers[i].aspect_map_size, sizeof(void *));
                mem_check(managers[i].aspect_map);
                managers[i].last_uuid = 0;
                return &managers[i];
            }
        }
        extend_manager_array();
    }
}
static void extend_manager_array(void)
{
    // increase space for managers linearly
    int prev_size = managers_array_size;
    managers_array_size += START_NUM_MANAGERS;
    managers = (Manager *) realloc(managers, sizeof(Manager) * managers_array_size);
    mem_check(managers);
    memset(managers + prev_size, 0, sizeof(Manager) * (managers_array_size - prev_size)); // should make dedicated nullifiers for structures
    /* for (int i = prev_size; i < managers_array_size; i++) { */
    /*     managers[i].type = 0; */
    /*     // ... */
    /* } */
}


static Manager *manager_of_type(AspectType type)
{
    //--- use this? if so, put in other functions too
    for (int i = 0; i < managers_array_size; i++) {
        if (managers[i].type == type) {
            return &managers[i];
        }
    }
    fprintf(stderr, "ERROR: attempted to access manager of type %d, but there is no manager for this type.\n", type);
    exit(EXIT_FAILURE);
}

static void *get_aspect_data(AspectID aspect)
{
    Manager *manager = manager_of_type(aspect.type);
    if (manager->aspect_map[aspect.map_index] == NULL) {
        return NULL;
    }
    if (((AspectProperties *) manager->aspect_map[aspect.map_index])->aspect_id.uuid != aspect.uuid) {
        return NULL;
    }
    return manager->aspect_map[aspect.map_index];
}


static void extend_aspect_map(Manager *manager)
{
    int prev_size = manager->aspect_map_size;
    manager->aspect_map_size += START_NUM_MANAGER_ASPECTS;
    manager->aspect_map = (void **) realloc(manager->aspect_map, sizeof(void *) * manager->aspect_map_size);
    mem_check(manager->aspect_map);
    memset(manager->aspect_map + prev_size, 0, sizeof(void *) * (manager->aspect_map_size - prev_size));
}

static AspectID create_aspect_id(AspectType type)
{
    AspectID id;
    Manager *manager = manager_of_type(type);
    id.uuid = ++ manager->last_uuid; // syntax ?
    id.type = type;
    while (1) {
        for (int i = 0; i < manager->aspect_map_size; i++) {
            if (manager->aspect_map[i] == NULL) {
                id.map_index = i;
                return id;
            }
        }
        extend_aspect_map(manager);
    }
    //--- have a default aspect manager
}
static void new_aspect(EntityID entity, AspectID aspect)
{
    //--- handling on manager_of_type or this?
    Manager *manager = manager_of_type(aspect.type);
    manager->new_aspect(manager, aspect);
    AspectProperties *properties = (AspectProperties *) manager->aspect_map[aspect.map_index];
    properties->entity_id = entity;
    properties->aspect_id = aspect;
    //--- have a default aspect manager
}

AspectID entity_add_aspect(EntityID entity, AspectType type)
{
    AspectID id = create_aspect_id(type);
    while (1) {
        for (int i = 0; i < entity_map[entity.map_index].num_aspects; i++) {
            if (entity_map[entity.map_index].aspects[i].type == NULL_ASPECT_TYPE) {
                entity_map[entity.map_index].aspects[i] = id;
                new_aspect(entity, id);
                return id;
            }
        }
        entity_extend_aspects(entity);
    }
}

static void entity_extend_aspects(EntityID entity)
{
    //- not checking nullness
    
    int prev_num_aspects = entity_map[entity.map_index].num_aspects;
    // increase space exponentially
    entity_map[entity.map_index].num_aspects *= 2;
    entity_map[entity.map_index].aspects = (AspectID *) realloc(entity_map[entity.map_index].aspects, sizeof(AspectID) * entity_map[entity.map_index].num_aspects);
    mem_check(entity_map[entity.map_index].aspects);
    // nullify the new space
    for (int i = prev_num_aspects; i < entity_map[entity.map_index].num_aspects; i++) {
        entity_map[entity.map_index].aspects[i].map_index = 0;
        entity_map[entity.map_index].aspects[i].uuid = 0;
        entity_map[entity.map_index].aspects[i].type = NULL_ASPECT_TYPE;
    }
}


void default_manager_new_aspect(Manager *manager, AspectID aspect)
{
    // Default manager just mallocs for the aspect data
    manager->aspect_map[aspect.map_index] = (void *) calloc(1, aspect_type_sizes[aspect.type]);
    mem_check(manager->aspect_map[aspect.map_index]);
}
void default_manager_destroy_aspect(Manager *manager, AspectID aspect)
{
    //--- checking
    //- no dynamic memory, default manager right now manages static aspects
    free(manager->aspect_map[aspect.map_index]);
}
void default_manager_aspect_iterator(Iterator *iterator)
{
}

//--------------------------------------------------------------------------------
// purely printing functions
//--------------------------------------------------------------------------------


void print_entities(void)
{
    printf("Entity printout:\n"); 
    for (int i = 0; i < entity_map_size; i++) {
        if (entity_map[i].aspects != NULL) { // currently, this is the way an entity map entry is "null"
            printf("Entity:\n");
            printf("\tuuid: %ld\n", entity_map[i].uuid);
            printf("\tnum_aspects: %d\n", entity_map[i].num_aspects);
        }
    }
}

void print_aspects_of_type(AspectType type)
{
    //- not using the manager aspect iterators
    printf("Aspects of type %d printout:\n", type);
    
    Manager *manager = manager_of_type(type);
    for (int i = 0; i < manager->aspect_map_size; i++) {
        if (manager->aspect_map[i] != NULL) {
            printf("Aspect:\n");
            AspectProperties *properties = (AspectProperties *) manager->aspect_map[i];
            printf("\tAspect ID:\n");
            printf("\t\tuuid: %ld\n", properties->aspect_id.uuid);
            printf("\t\tmap_index: %d\n", properties->aspect_id.map_index);
            printf("\t\ttype: %d\n", properties->aspect_id.type);
            printf("\tEntity ID:\n");
            printf("\t\tuuid: %ld\n", properties->entity_id.uuid);
            printf("\t\tmap_index: %d\n", properties->entity_id.map_index);
        }
    }
}


