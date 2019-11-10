/* 
 * Linked list implementation of an entity system. This is not very efficient, but for now,
 * it is fine to just have an entity system working.
 *
 * --- fixed-width arrays of components/objects at the universe level is a major restriction. Make universe not an "entity" but have the entities form
 *  a forest instead?
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "helper_definitions.h"
#include "shapes.h"
#include "entity.h"

static bool ENTITY_MODEL_ACTIVE = false;
static EntityNode universe_node;
static Entity *universe;
static EntityID current_entity_id;
static ComponentID current_component_id;

static EntityNode *entity_nodes;
static EntityNode *last_entity_node;
static ComponentNode *component_nodes;
static ComponentNode *last_component_node;


void print_entity_tree(void)
{
    _print_entity_tree(universe, 0);
}
static void _print_entity_tree(Entity *entity, int indent_level)
{
    /* Recursively traverse the tree of entities below the given entity (universe if NULL),
     * and print it out nicely.
     */
    for (int i = 0; i < indent_level; i++) {
        printf("  ");
    }
    printf("%s<%ld>", entity->name, entity->id);
    printf(" [");
    for (int i = 0; i < MAX_ENTITY_COMPONENTS; i++) {
        if (entity->components[i] != NULL_COMPONENT_ID) {
            Component *component = get_component(entity->components[i]);
            printf("%s", component->name);
            printf(", ");
        }
    }
    printf("]");
    printf("\n");

    for (int i = 0; i < MAX_ENTITY_CHILDREN; i++) {
        if (entity->children[i] != NULL_ENTITY_ID) {
            Entity *child_entity = get_entity(entity->children[i]);
            _print_entity_tree(child_entity, indent_level + 1);
        }
    }
}

static ComponentID _entity_add_component(EntityID entity_id, char *name, ComponentType component_type)
{
    ComponentNode *new_component_node = (ComponentNode *) calloc(1, sizeof(ComponentNode));
    mem_check(new_component_node);

    Component *new_component = &new_component_node->component;
    new_component->type = component_type;
    new_component->entity_id = entity_id;
    new_component->id = new_component_id();
    strncpy(new_component->name, name, MAX_COMPONENT_NAME_LENGTH);

    Entity *entity = get_entity(entity_id);
    bool connected_the_component = false;
    for (int i = 0; i < MAX_ENTITY_COMPONENTS; i++) {
        if (entity->components[i] == NULL_COMPONENT_ID) {
            entity->components[i] = new_component->id;
            connected_the_component = true;
            break;
        }
    }
    if (!connected_the_component) {
        fprintf(stderr, "ERROR: not enough space to add a component to entity %s.\n", entity->name);
        exit(EXIT_FAILURE);
    }

    new_component_node->next = NULL;
    last_component_node->next = new_component_node;
    last_component_node = new_component_node;

    return new_component->id;
}

Entity *get_entity(EntityID entity_id)
{
    EntityNode *cur = entity_nodes;
    while (true) {
        if (cur->entity.id == entity_id) {
            return &cur->entity;
        }
        if (cur->next == NULL) {
            break;
        }
        cur = cur->next;
    }
    return NULL;
}
Component *get_component(ComponentID component_id)
{
    ComponentNode *cur = component_nodes;
    while (true) {
        if (cur->component.id == component_id) {
            return &cur->component;
        }
        if (cur->next == NULL) {
            break;
        }
        cur = cur->next;
    }
    return NULL;
}

EntityID new_entity_id()
{
    /* Retrieve the next available entity ID.
     */
    return current_entity_id ++;
}
ComponentID new_component_id()
{
    /* Retrieve the next available component ID.
     */
    return current_component_id ++;
}


static void entity_add_child(Entity *entity, Entity *child)
{
    /* Helper function
     */
    for (int i = 0; i < MAX_ENTITY_CHILDREN; i++) {
        if (entity->children[i] == NULL_ENTITY_ID) {
            entity->children[i] = child->id;
            return;
        }
    }
    fprintf(stderr, "ERROR: not enough room to add a child to entity %s.\n", entity->name);
    exit(EXIT_FAILURE);
}
EntityID create_entity(EntityID parent_id, char *name)
{
    return ptr_create_entity(parent_id, name)->id;
}
static Entity *ptr_create_entity(EntityID parent_id, char *name)
{
    EntityNode *new_entity_node = (EntityNode *) calloc(1, sizeof(EntityNode));
    mem_check(new_entity_node);

    Entity *new_entity = &new_entity_node->entity;
    strncpy(new_entity->name, name, MAX_ENTITY_NAME_LENGTH);
    new_entity->id = new_entity_id();
    new_entity->parent_id = parent_id;

    Entity *parent = get_entity(parent_id);
    entity_add_child(parent, new_entity);

    new_entity_node->next = NULL;
    last_entity_node->next = new_entity_node;
    last_entity_node = new_entity_node;

    return new_entity;
}

void init_entity_model()
{
    /* Initialize the entity model. This should:
     *     - initialize the universe entity
     *     - initialize the linked lists of entities and components
     *     - activate the active flag
     */
    if (ENTITY_MODEL_ACTIVE) {
        fprintf(stderr, "ERROR: entity model is already active.\n");
        exit(EXIT_FAILURE);
    }

    universe = &universe_node.entity;
    universe->id = 1;
    strncpy(universe->name, "universe", MAX_ENTITY_NAME_LENGTH);

    universe_node.next = NULL;
    entity_nodes = &universe_node;
    last_entity_node = &universe_node;

    ENTITY_MODEL_ACTIVE = true;
}
void close_entity_model()
{
    /* Close/terminate the entity model. This should:
     *     - make sure all used memory is freed
     *     - free the linked lists of entities and components
     *     - deactivate the active flag
     */

    if (!ENTITY_MODEL_ACTIVE) {
        fprintf(stderr, "ERROR: entity model is not active, cannot close.\n");
        exit(EXIT_FAILURE);
    }
    // --- destroy all entities
    // --- free linked lists
    ENTITY_MODEL_ACTIVE = false;
}

