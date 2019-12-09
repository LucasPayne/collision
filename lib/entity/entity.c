/*-------------------------------------------------------------------------------- 
   Definitions for the entity model.
   See the header for details.
--------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "helper_definitions.h"
#include "entity.h"

// Small values for testing
static bool entity_model_active = false;
#define ENTITY_MAP_START_SIZE 12
static EntityMapEntry *entity_map = NULL;
static unsigned int entity_map_size = 0;

static UUID last_uuid = 0;

static uint32_t g_num_aspect_types = 0;
static uint32_t g_managers_length = 0;
static Manager *g_managers = NULL;
#define START_NUM_MANAGERS 3
#define START_NUM_MANAGER_ASPECTS 5

// Static declarations
//--------------------------------------------------------------------------------
static void entity_extend_aspects(EntityID entity);
static void new_aspect(EntityID entity, AspectID aspect);
static AspectID create_aspect_id(AspectType type);
static void extend_aspect_map(Manager *manager);
static AspectID *get_entity_aspects(EntityID entity);
static void extend_entity_map(void);
//--------------------------------------------------------------------------------

void init_entity_model(void)
{
    if (entity_model_active) {
        fprintf(stderr, ERROR_ALERT "Entity model is already active.\n");
        exit(EXIT_FAILURE);
    }
    // The entity map is a global dynamic array which is indexed into by the map_index component of entity IDs.
    entity_map_size = ENTITY_MAP_START_SIZE;
    entity_map = (EntityMapEntry *) calloc(entity_map_size, sizeof(EntityMapEntry));
    mem_check(entity_map);

    // The managers array is a dynamic array meant to be filled at the start of the application through the new_manager macro. Managers
    // and aspect types are tightly associated, and both parts are made at once and encapsulated by a Manager structure.
    // This contains aspect type information and functions for managing this type of aspect.
    g_managers_length = START_NUM_MANAGERS;
    g_managers = (AspectType *) calloc(g_managers_length, sizeof(Manager));
    mem_check(g_managers);
    g_num_aspect_types = 0;

    entity_model_active = true;
}

static void extend_entity_map(void)
{
    // Linearly increase the amount of space
    int previous_size = entity_map_size;
    entity_map_size += ENTITY_MAP_START_SIZE;
    entity_map = (EntityMapEntry *) realloc(entity_map, sizeof(EntityMapEntry) * entity_map_size);
    mem_check(entity_map);
    //- I did this for the other array extenders, but apparently not having it here was a source
    // of a bug. Must initialize new space on realloc!
    memset(entity_map + previous_size, 0, sizeof(EntityMapEntry) * (entity_map_size - previous_size));
}


EntityID new_entity(int start_num_aspects)
{
    /* Returns a new entity ID for a new entity with the given number of aspects available. This need not be set exactly
     * but a caller may anticipate how many aspects an entity will be given and pass this function that number.
     */
    // Create a new entity ID. This could be another function, but currently this is only needed here.
    EntityID id;
    id.uuid = ++last_uuid;
    while (1) {
        bool found_one = false;
        for (int i = 0; i < entity_map_size; i++) {
            if (entity_map[i].aspects == NULL) {
                id.map_index = i;
                found_one = true;
                break;
            }
        }
        if (found_one) break;
        extend_entity_map();
    }

    // The created entity ID indexes into the global entity map. Initialize this entity map entry
    // and attach to it a dynamic aspect list of length start_num_aspects.
    entity_map[id.map_index].uuid = id.uuid;
    entity_map[id.map_index].num_aspects = start_num_aspects;
    entity_map[id.map_index].aspects = (AspectID *) malloc(sizeof(AspectID) * start_num_aspects);
    mem_check(entity_map[id.map_index].aspects);
    // Null-initialize the available starting aspects for this entity.
    for (int i = 0; i < start_num_aspects; i++) {
        entity_map[id.map_index].aspects[i].map_index = 0;
        entity_map[id.map_index].aspects[i].uuid = 0;
        entity_map[id.map_index].aspects[i].type = 0;
    }
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

Manager *_new_manager(AspectType *type_pointer,
                      size_t size,
                      char *type_name,
                      void (*new_aspect)(Manager *, AspectID),
                      void (*destroy_aspect)(Manager *, AspectID),
                      void (*aspect_iterator)(Iterator *),
                      void (*serialize) (FILE *, void *))
{
    if (strlen(type_name) > MAX_MANAGER_NAME_LENGTH) {
        fprintf(stderr, ERROR_ALERT "Attempted to create a new aspect type with name \"%s\". This name is too long. The maximum is set to %d.\n", type_name, MAX_MANAGER_NAME_LENGTH);
        exit(EXIT_FAILURE);
    }
    if (g_num_aspect_types >= g_managers_length) {
        g_managers_length ++;
        g_managers = (Manager *) realloc(g_managers, g_managers_length * sizeof(Manager));
        mem_check(g_managers);
    }
    // Initialize the manager information.
    Manager *manager = &g_managers[g_num_aspect_types];
    manager->new_aspect = new_aspect;
    manager->destroy_aspect = destroy_aspect;
    manager->aspect_iterator = aspect_iterator;
    manager->serialize = serialize;
    manager->aspect_map_size = START_NUM_MANAGER_ASPECTS;
    manager->aspect_map = (void **) calloc(manager->aspect_map_size, sizeof(void *));
    mem_check(manager->aspect_map);
    manager->last_uuid = 0;
    // Initialize the aspect type information.
    manager->size = size;
    manager->type_id = g_num_aspect_types;
    strncpy(manager->name, type_name, MAX_MANAGER_NAME_LENGTH);
    // This is set because this function is called from a macro expanding to give type information. <Name>_TYPE_ID must be defined as a global on the caller's side, meaning
    // that subsequent macros which use the type-name symbol can expand to <Name>_TYPE_ID, which should be defined and set to a unique type ID that indexes into the global
    // aspect type information table.
    *type_pointer = manager->type_id;
    g_num_aspect_types ++;

    return manager;
}

Manager *manager_of_type(AspectType type)
{
    //--- use this? if so, put in other functions too
    for (int i = 0; i < g_num_aspect_types; i++) {
        if (g_managers[i].type_id == type) {
            return &g_managers[i];
        }
    }
    fprintf(stderr, ERROR_ALERT "Attempted to access manager of type %d, but there is no manager for this type.\n", type);
    exit(EXIT_FAILURE);
}

// only non-static because a macro expands to it ...
void *get_aspect_data(AspectID aspect)
{
    Manager *manager = manager_of_type(aspect.type);
    if (manager->aspect_map[aspect.map_index] == NULL) {
        return NULL;
    }
    if (((AspectProperties *) manager->aspect_map[aspect.map_index])->aspect_id.uuid != aspect.uuid) {
        // stale map entry, non-matching UUIDs
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
    // Initialize the extension to zero
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
    if (manager == NULL) {
        fprintf(stderr, ERROR_ALERT "Attempted to access a manager of an invalid aspect type, or something went wrong and getting this manager has failed.\n");
        exit(EXIT_FAILURE);
    }
    if (manager->new_aspect == NULL) {
        fprintf(stderr, ERROR_ALERT "Attempted to create a new aspect under a manager with no new_aspect function.\n");
        exit(EXIT_FAILURE);
    }
    manager->new_aspect(manager, aspect);
    AspectProperties *properties = (AspectProperties *) manager->aspect_map[aspect.map_index];
    properties->entity_id = entity;
    properties->aspect_id = aspect;
    //--- have a default aspect manager? Or just explicitly make a default manager for each aspect which can just malloc everywhere?
}

AspectID _entity_add_aspect(EntityID entity, AspectType type)
{
    AspectID id = create_aspect_id(type);
    while (1) {
        for (int i = 0; i < entity_map[entity.map_index].num_aspects; i++) {
            if (entity_map[entity.map_index].aspects[i].uuid == 0) {
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
        entity_map[entity.map_index].aspects[i].type = 0;
    }
}


void default_manager_new_aspect(Manager *manager, AspectID aspect)
{
    // Default manager just mallocs for the aspect data (and zero initializes)
    manager->aspect_map[aspect.map_index] = (void *) calloc(1, g_managers[aspect.type].size);
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
    Manager *manager = iterator->data1.ptr_val;
    int map_index = iterator->data2.int_val;
BEGIN_COROUTINE_SA(iterator)
coroutine_start:
    map_index = 0;
    iterator->data2.int_val = 0;
    iterator->coroutine_flag = COROUTINE_A;
coroutine_a:
    iterator->data2.int_val ++;
    while (1) {
        if (iterator->data2.int_val >= manager->aspect_map_size) {
            iterator->val = NULL;
            return;
        }
        if (manager->aspect_map[map_index] != NULL) {
            iterator->val = manager->aspect_map[map_index];
            return;
        }
        iterator->data2.int_val ++;
    }
}

//--------------------------------------------------------------------------------
// purely printing functions
//--------------------------------------------------------------------------------


void print_entities(void)
{
    printf("Entity printout:\n"); 
    for (int i = 0; i < entity_map_size; i++) {
        printf("checking map index: %d, entity_map_size : %d, uuid: %ld\n", i, entity_map_size, entity_map[i].uuid);
        if (entity_map[i].aspects != NULL && entity_map[i].uuid != 0) { // how is an entity entry null?
            EntityID entity;
            entity.uuid = entity_map[i].uuid;
            entity.map_index = i;
            print_entity(entity);
        }
    }
}

void print_entity(EntityID entity)
{
    AspectID *aspects = get_entity_aspects(entity);
    printf("Entity %ld:\n", entity.uuid);
    if (aspects == NULL) {
        printf("ENTITY DOES NOT EXIST.\n");
        return;
    }
    int num_aspects = entity_map[entity.map_index].num_aspects;
    printf("\tnum_aspects: %d\n", num_aspects);

    for (int i = 0; i < num_aspects; i++) {
        if (aspects[i].type != 0) {
            printf("Aspect type: %d\n", aspects[i].type);
            Manager *manager = manager_of_type(aspects[i].type);
            if (manager->serialize != NULL) {
                printf("Serialization:\n");
                manager->serialize(stdout, get_aspect_data(aspects[i]));
            }
        }
    }
}

void _print_aspects_of_type(AspectType type)
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
            if (manager->serialize != NULL) {
                printf("\t\tSerialization:\n");
                manager->serialize(stdout, manager->aspect_map[i]);
            }
        }
    }
}

void *_get_aspect_type(EntityID entity, AspectType type)
{
    AspectID *aspects = get_entity_aspects(entity);
    if (aspects == NULL) {
        fprintf(stderr, ERROR_ALERT "Attempted to get an aspect of type %d on non-existent entity with UUID %ld.\n", type, entity.uuid);
        exit(EXIT_FAILURE);
    }
    int num_aspects = entity_map[entity.map_index].num_aspects;
    for (int i = 0; i < num_aspects; i++) {
        if (aspects[i].type == type) {
            return get_aspect_data(aspects[i]);
        }
    }
    fprintf(stderr, ERROR_ALERT "Attempted to get a non-existent aspect of type %d on existent entity with UUID %ld.\n", type, entity.uuid);
    exit(EXIT_FAILURE);
}

void print_aspect_types(void)
{
    printf("Aspect types (%d):\n", (int) g_num_aspect_types);
    for (int i = 0; i < g_num_aspect_types; i++) {
        printf("name: %s\n", g_managers[i].name);
        printf("type_id: %d\n", (int) g_managers[i].type_id);
        printf("size: %ld\n", g_managers[i].size);
    }
}
