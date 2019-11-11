/*
 * Entity (or gameobject) model.
 *
 * Entities and components are referenced by IDs. An invalid ID is assumed to be destroyed, since it cannot be found,
 * and attempts should only be made to get entities of IDs which have been allocated before. So, ID's are never reallocated.
 * Also, entities are never realloced. Part of the point of this is that while pointers can be trusted right after getting a 
 * reference, there would be no facilities to go and find dangling pointers when an entity is destroyed. So, the ID just gives a NULL entity,
 * meaning that if the ID used has been allocated before, that entity has been destroyed.
 *
 * todo:
 *  typedef function types
 *  heirarchical transform (update this as part of "bookkeeping" of entity system?)
 *
 * Dead children: currently num_children does not actually mean the number of alive children. Dead children are NULL.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "helper_definitions.h"
#include "shapes.h"
#include "entity.h"

#define DEBUG 1
#define entity_model_check()\
    if (!entity_model_active) {\
        fprintf(stderr, "ERROR: trying to use entity functions while the entity model is not initialized.\n");\
        exit(EXIT_FAILURE);\
    }

static Entity2D universe; // root "/", EntityID 1 (0 is reserved)
static bool entity_model_active = false;
static EntityID CURRENT_ENTITY_ID = 0;
    // unsigned long ints can be so large that I don't think it matters to even check overflow, and ID's could be allocated with ++
static ComponentID CURRENT_COMPONENT_ID = 0;

EntityID new_entity_id()
{
    return CURRENT_ENTITY_ID ++;
}
ComponentID new_component_id()
{
    return CURRENT_COMPONENT_ID ++;
}

Entity2D *get_entity(EntityID id)
{
    // currently just this for now.
    return get_sub_entity(id, &universe);
}
Entity2D *get_sub_entity(EntityID id, Entity2D *entity)
{
    if (entity->id == id) {
        return entity;
    }
    for (int i = 0; i < entity->num_children; i++) {
        if (entity->children[i] != NULL) {
            Entity2D *got = get_sub_entity(id, entity->children[i]);
            if (got != NULL) {
                return got;
            }
        }
    }
    return NULL;
}

Component *get_component(ComponentID id)
{
    return 
}



void print_entity_tree(Entity2D *entity)
{
#if DEBUG
    entity_model_check();
#endif
    _print_entity_tree(entity, 0);
}
static void _print_entity_tree(Entity2D *entity, int indent_level)
{
#if DEBUG
    entity_model_check();
#endif
    /* Recursively traverse the tree of entities below the given entity (universe if NULL),
     * and print it out nicely.
     */
    if (entity == NULL) {
        entity = &universe;
    }
    for (int i = 0; i < indent_level; i++) {
        printf("  ");
    }
    printf("%s<%ld>", entity->name, entity->id);
    if (entity->num_children != 0) {
        printf("/");
    }
    printf(" (%.2lf, %.2lf : %.2lf)", entity->transform.position.x,
                                      entity->transform.position.y,
                                      entity->transform.rotation);
    printf(" [");
    for (int i = 0; i < entity->num_components; i++) {
        printf("%s", entity->components[i]->name);
        if (i != entity->num_components - 1) {
            printf(", ");
        }
    }
    printf("]");
    printf("\n");
    for (int i = 0; i < entity->num_children; i++) {
        _print_entity_tree(entity->children[i], indent_level + 1);
    }
}


// If new stuff is added, define defaults ("zeroes") here to not mess with anything else.
void zero_init_entity(Entity2D *entity)
{
#if DEBUG
    entity_model_check();
#endif
    entity->transform.position.x = 0;
    entity->transform.position.y = 0;
    entity->transform.rotation = 0;
    entity->marked_for_destruction = false;
    entity->name[0] = '\0';
    entity->id = 0;
    entity->init = NULL;
    
    entity->parent = NULL;
    entity->num_children = 0;
    entity->child_space = 0;
    entity->children = NULL;
    entity->num_components = 0;
    entity->component_space = 0;
    entity->components = NULL;

    entity->data = NULL;
}

void entity_add_child(Entity2D *parent, Entity2D *child)
{
#if DEBUG
    entity_model_check();
#endif
    // Expand space for children if a new one would be too much.
    if (parent->num_children == parent->child_space) {
        parent->child_space *= 2;
        parent->children = (Entity2D **) realloc(parent->children, sizeof(Entity2D *) * parent->child_space);
        mem_check(parent->children);
    }

    parent->children[parent->num_children] = child;
    parent->num_children ++;
}


EntityID create_entity(EntityID parent_id,
                       char *name,
                       void (*init) (struct Entity2D_s *),
                       double position_x,
                       double position_y,
                       double rotation)
{
#if DEBUG
    entity_model_check();
#endif
    Entity2D *parent = get_entity(parent_id);
    check_entity(parent);

    Entity2D *new_entity = (Entity2D *) malloc(sizeof(Entity2D));
    mem_check(new_entity);
    zero_init_entity(new_entity);

    new_entity->id = new_entity_id();

    new_entity->child_space = START_NUM_CHILDREN;
    EntityID *new_entity_children = (EntityID *) malloc(sizeof(EntityID) * new_entity->child_space);
    mem_check(new_entity_children);
    new_entity->children = new_entity_children;

    new_entity->component_space = START_NUM_COMPONENTS;
    ComponentID *new_entity_components = (ComponentID *) malloc(sizeof(ComponentID) * new_entity->component_space);
    mem_check(new_entity_components);
    new_entity->components = new_entity_components;

    strncpy(new_entity->name, name, MAX_ENTITY_NAME_LENGTH);

    new_entity->init = init;
    new_entity->transform.position.x = position_x;
    new_entity->transform.position.y = position_y;
    new_entity->transform.rotation = rotation;

    new_entity->data = NULL; 

    new_entity->parent = parent;
    entity_add_child(parent, new_entity);

    if (new_entity->init != NULL) {
        new_entity->init(new_entity);
    }
    return new_entity->id;
}


void free_entity(Entity2D *entity)
{
#if DEBUG
    entity_model_check();
    if (entity == NULL) {
        fprintf(stderr, "ERROR: Attempted to free a null entity.\n");
        exit(EXIT_FAILURE);
    }
#endif
    free(entity->data);
    free(entity->children);
    free(entity->components);
    free(entity);
}

void destroy_entity(EntityID entity_id)
{
    /* Prepares entity for destruction. Destruction should not be in-frame, but at a designated step.
     * This means that pointer references should never last for more than a frame. (?..)
     */
    Entity2D *entity = get_entity(entity_id);
    if (entity == NULL) {
        // destroying does nothing if the entity doesn't exist
        return;
    }
    entity->marked_for_destruction = true;
}
static void annihilate_entity(EntityID entity_id)
{
    /* annihilate : do not use, except for prepared destructions. Instant effect. */
#if DEBUG
    entity_model_check();
    if (entity == NULL) {
        fprintf(stderr, "ERROR: Attempted to annihilate a null entity.\n");
        exit(EXIT_FAILURE);
    }
#endif
    Entity2D *entity = get_entity(entity_id);
    if (entity == NULL) {
        // annihilating does nothing if the entity doesn't exist
        return;
    }
    // NULL parent's reference
    if (entity->parent_id != ENTITY_ID_NULL) { // should only be null for universe anyway
        EntityID *parent = get_entity(entity->parent_id);
        // entity_check?
        for (int i = 0; i < entity->parent->num_children; i++) {
            if (entity->parent->child_ids[i] == entity->id) {
                entity->parent->child_ids[i] = ENTITY_ID_NULL;
                break;
            }
        }
    }
    for (int i = 0; i < entity->num_children; i++) {
        if (entity->child_ids[i] != ENTITY_ID_NULL) {
            destroy_entity(entity->child_ids[i]);
            entity->children[i] = ENTITY_ID_NULL;
        }
    }
    for (int i = 0; i < entity->num_components; i++) {
        if (entity->components[i] != COMPONENT_ID_NULL) {
            destroy_component(entity->component_ids[i]);
            entity->component_ids = COMPONENT_ID_NULL;
        }
    }
    free(entity->data);
    free(entity->children);
    free(entity->components);
    free(entity);
}

void destroy_component(ComponentID component_id)
{
    get_component

}
static void annihilate_component(ComponentID component_id)
{
    free(component->data);
}

void init_entity_model(void)
{
    if (entity_model_active) {
        fprintf(stderr, "ERROR: Entity model is already initialized.\n");
        exit(EXIT_FAILURE);
    }
    entity_model_active = true;

    CURRENT_ENTITY_ID = ENTITY_UNIVERSE; // 0 is reserved
    CURRENT_COMPONENT_ID = COMPONENT_ID_START;

    zero_init_entity(&universe);
    strncpy(universe.name, "universe", MAX_ENTITY_NAME_LENGTH);
    universe.parent = NULL;

    universe.id = new_entity_id();

    universe.child_space = UNIVERSE_START_NUM_CHILDREN;
    universe.children = (Entity2D **) malloc(sizeof(Entity2D *) * universe.child_space);
    mem_check(universe.children);
}

void close_entity_model(void)
{
    if (!entity_model_active) {
        fprintf(stderr, "ERROR: entity model is not yet initialized, cannot close it.\n");
        exit(EXIT_FAILURE);
    }

    /* for (int i = 0; i < universe.num_children; i++) { */
    /*     if (universe.children[i] != NULL) { */
    /*         destroy_entity(universe.children[i]); */
    /*     } */
    /* } */

    entity_model_active = false;
}


void update_entity_model(void)
{
#if DEBUG
    entity_model_check();
#endif
    _update_entity_model(&universe);
}
static void _update_entity_model(Entity2D *entity)
{
    for (int i = 0; i < entity->num_children; i++) {
        if (entity->children[i] != NULL) {
            _update_entity_model(entity->children[i]);
        }
    }
    if (entity->marked_for_destruction) {
        annihilate_entity(entity);
    }
}


//================================================================================
// Components
//================================================================================

ComponentID entity_add_component(EntityID entity_id, ComponentType component_type, char *name)
{
    Entity2D *entity = get_entity(entity_id);

    if (entity->num_components == entity->component_space) {
        entity->component_space *= 2;
        entity->components = (Component **) malloc(sizeof(Component *) * entity->component_space);
        mem_check(entity->components);
    }
    Component *new_component = (Component *) malloc(sizeof(Component));
    mem_check(new_component);

    strncpy(new_component->name, name, MAX_COMPONENT_NAME_LENGTH);

    init_component(new_component, entity, component_type);
    new_component->id = new_component_id();

    entity->components[entity->num_components] = new_component;
    entity->num_components ++;

    return new_component->id;
}


void init_component(Component *component, Entity2D *entity, ComponentType component_type)
{

}

#undef DEBUG
