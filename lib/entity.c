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

    Probably could reorganize create/new/allocate id stuff
--------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "helper_definitions.h"
#include "entity.h"
#include "iterator.h"

typedef MapIndex uint16_t;
typedef UUID uint64_t; // ?

#define NULL_ENTITY_ID { -1, 0 } // ???
typedef struct EntityID_s {
    MapIndex map_index;
    UUID uuid;
} EntityID;

static bool entity_model_active = false;
// could make this dynamic, I don't think there would be anything wrong with doing that
#define ENTITY_MAP_SIZE 1024
static Entity *entity_map[ENTITY_MAP_SIZE];

static UUID last_uuid = 0;

void init_entity_model(void)
{
    if (entity_model_active) {
        fprintf(stderr, "ERROR: entity model is already active.\n");
        exit(EXIT_FAILURE);
    }
    memcpy(entity_map, NULL, ENTITY_MAP_SIZE);
    entity_model_active = true;
}

static EntityID create_entity_id(void)
{
    /* Creating a new entity id does not actually do anything with the map.
     * No objects are created and no map index is used up. However, a uuid is used,
     * but I think this is fine. This just gives an EntityID that can be used to create
     * an entity straight after calling.
     */
    EntityID id;
    id.uuid = ++last_uuid;
    for (int i = 0; i < ENTITY_MAP_SIZE; i++) {
        if (entity_map[i] == NULL) {
            id.map_index = i;
            return id;
        }
    }
    fprintf(stderr, "ERROR: Attempted to create a new entity ID. The entity map is full.\n");
    exit(EXIT_FAILURE);
}

Entity *get_entity(EntityID entity_id)
{
    if (entity_map[entity_id.map_index] == NULL) {
        fprintf("ERROR: Attempted to get an entity with invalid id. Possibly bad ID or the object has been destroyed.\n");
        exit(EXIT_FAILURE);
    }
    if (entity_map[entity_id.map_index]->id.uuid != entity_id.uuid) {
        fprintf("ERROR: Attempted to get an entity with invalid id. Possibly bad ID or the object has been destroyed.\n");
        exit(EXIT_FAILURE);
    }
    return entity_map[entity_id.map_index];
}

static Entity *new_entity(EntityID id, char *name)
{
    /* Given a newly created entity ID, allocate memory for an entity
     * and update the map structure.
     */
    Entity *entity = (Entity *) malloc(sizeof(Entity));
    mem_check(entity);
    entity_map[id.map_index] = entity;

    entity->id = id;
    strncpy(name, entity->name, MAX_ENTITY_NAME_LENGTH);
    entity->parent_id = NULL_ENTITY_ID;
    entity->on = true;
    for (int i = 0; i < MAX_ENTITY_CHILDREN; i++) {
        entity->children[i] = NULL_ENTITY_ID;
    }
    entity->parent_id = NULL_ENTITY_ID;
    // ----
    /* for (int i = 0; i < MAX_ENTITY_COMPONENTS; i++) { */
    /*     entity->components[i] = NULL_COMPONENT_ID; */
    /* } */
}


EntityID create_entity(EntityID parent_id, char *name)
{
    Entity *parent = get_entity(parent_id);
    EntityID new_id = create_entity_id();
    Entity *entity = new_entity(new_id, name);
    entity->parent_id = parent_id;
    
    // Add the child to the parent entity
    if (parent->num_children == MAX_ENTITY_CHILDREN) {
        fprintf(stderr, "ERROR: attempted to create a new entity while the parent entity has too many children.\n");
        exit(EXIT_FAILURE);
    }
    parent->children[num_children ++ ] = entity->id;

    return entity->id;
}


//--------------------------------------------------------------------------------
/* bool destroy_entity(EntityID entity_id) */
/* { */
/*     Entity *entity = get_entity(entity_id); */
/* } */

