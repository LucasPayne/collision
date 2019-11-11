/* 
 * Linked list implementation of an entity system. This is not very efficient, but for now,
 * it is fine to just have an entity system working. (it is not working yet)
 * Also, children and component IDs are stored in fixed-size arrays in the structures in this implementation.
 *
 * --- fixed-width arrays of components/objects at the universe level is a major restriction. Make universe not an "entity" but have the entities form
 *  a forest instead?
 *
 * The most important thing is the design of the interface. The ID system,
 * what calls are made to prepare an entity with components.
 *
 * Comment descriptions even should only be in header files.
 * The linked list model can then be phased out, some memory management stuff included, with no change
 * to the design of the interface. Separate interface functions from module-specific "helper" functions.
 * --- how to do this with a single link?
 *
 *  Does a component need its type? Why not keep it with the id handle on an entity?
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "helper_definitions.h"
#include "shapes.h"
#include "entity.h"
#include "iterator.h"

//================================================================================
// Linked-list implementation of entity and component pool
//================================================================================
typedef struct EntityNode_structure {
    struct EntityNode_structure *next;
    Entity entity;
} EntityNode;

typedef struct ComponentNode_structure {
    struct ComponentNode_structure *next;
    Component component;
} ComponentNode;

static EntityNode *entity_nodes;
static EntityNode *last_entity_node;
static ComponentNode *component_nodes;
static ComponentNode *last_component_node;

static EntityNode universe_node;
static ComponentNode universe_component_node;
//================================================================================

static System systems[MAX_NUM_SYSTEMS];
static bool systems_active[MAX_NUM_SYSTEMS];

static bool ENTITY_MODEL_ACTIVE = false;
static Entity *universe;
static EntityID current_entity_id;
static ComponentID current_component_id;


void print_entity_list(void)
{
    EntityNode *cur = entity_nodes;
    do {
        printf("%s", cur->entity.name);
        printf(", ");
        cur = cur->next;
    } while (cur != NULL);
    printf("\n");
}

void print_entity_tree(void)
{
    /* Recursively traverse the tree of entities below the universe
     * and print it out nicely.
     */
    _print_entity_tree(universe, 0);
}
static void _print_entity_tree(Entity *entity, int indent_level)
{
    for (int i = 0; i < indent_level; i++) {
        printf("  ");
    }
    printf("%s<%ld>", entity->name, entity->id);
    printf(" [");
    for (int i = 0; i < MAX_ENTITY_COMPONENTS; i++) {
        if (entity->components[i] != NULL_COMPONENT_ID) {
            Component *component = get_component(entity->components[i]);
            printf("%s<%ld>", component->name, component->id);
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

#define TRACING 0
ComponentID entity_add_component_from_types(EntityID entity_id, char *name, ComponentType component_type, size_t component_size)
{
    return entity_add_component_from_types_get(entity_id, name, component_type, component_size)->id;
}
Component *entity_add_component_from_types_get(EntityID entity_id, char *name, ComponentType component_type, size_t component_size)
{
#if TRACING
    printf("Adding component %s to entity with ID %ld ...\n", name, entity_id);
#endif
    ComponentNode *new_component_node = (ComponentNode *) calloc(1, component_size + (sizeof(ComponentNode) - sizeof(Component)));
        // hmm ...
    mem_check(new_component_node);
    Component *new_component = &new_component_node->component;
    new_component->type = component_type;
    new_component->entity_id = entity_id;
    new_component->id = new_component_id();
    strncpy(new_component->name, name, MAX_COMPONENT_NAME_LENGTH);

    Entity *entity = get_entity(entity_id);
#if TRACING
    printf("Connecting component \"%s\" to entity \"%s\" ...\n", name, entity->name);
#endif
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
#if TRACING
    printf("Component connected successfully.\n");
    printf("Adding to linked list ...\n");
#endif
    new_component_node->next = NULL;
    last_component_node->next = new_component_node;
    last_component_node = new_component_node;
#if TRACING
    printf("Added to linked list successfully.\n");
#endif

    return new_component;
}
#undef TRACING

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

static EntityID new_entity_id()
{
    /* Retrieve the next available entity ID.
     */
    return current_entity_id ++;
}
static ComponentID new_component_id()
{
    /* Retrieve the next available component ID.
     */
    return current_component_id ++;
}


static void entity_add_child(Entity *entity, Entity *child)
{
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
    universe->id = UNIVERSE_ID;
    strncpy(universe->name, "universe", MAX_ENTITY_NAME_LENGTH);

    current_entity_id = UNIVERSE_ID + 1;

    universe_node.next = NULL;
    entity_nodes = &universe_node;
    last_entity_node = &universe_node;

    // This is very, very bad.
    // start the linked list of components with a dummy component
    Component *universe_component = &universe_component_node.component;
    strncpy(universe_component->name, "universe component", MAX_COMPONENT_NAME_LENGTH);
    universe_component->id = 1;
    current_component_id = 2;
    universe_component->type = 0;
    universe_component->entity_id = UNIVERSE_ID;
    universe_component_node.next = NULL;

    component_nodes = &universe_component_node;
    last_component_node = &universe_component_node;

    for (int i = 0; i < MAX_NUM_SYSTEMS; i++) {
        systems_active[i] = false;
    }

    ENTITY_MODEL_ACTIVE = true;
}

void update_entity_model()
{
    for (int i = 0; i < MAX_NUM_SYSTEMS; i++) {
        if (systems_active[i]) {
            update_system(&systems[i]);
        }
    }
}
static void update_system(System *system)
{
    if (system->update != NULL) {
        system->update(system);
    }
}

#define TRACING 1
System *add_system(char *name, void (*init) (System *), void (*update) (System *), void (*close) (System *))
{
#if TRACING
    printf("Adding system named \"%s\" ...\n", name);
#endif

    for (int i = 0; i < MAX_NUM_SYSTEMS; i++) {
        if (!systems_active[i]) {
            strncpy(systems[i].name, name, MAX_SYSTEM_NAME_LENGTH);
            systems[i].init = init;
            systems[i].update = update;
            systems[i].close = close;

            if (systems[i].init != NULL) {
                systems[i].init(&systems[i]); // ?
            }

            systems_active[i] = true;
            return &systems[i];
        }
    }
    fprintf(stderr, "ERROR: not enough space to add a new system.\n");
    exit(EXIT_FAILURE);
}
#undef TRACING


void close_entity_model()
{
    /* Close/terminate the entity model. This should:
     *     - close all the systems
     *     - make sure all used memory is freed
     *     - free the linked lists of entities and components
     *     - deactivate the active flag
     */
    if (!ENTITY_MODEL_ACTIVE) {
        fprintf(stderr, "ERROR: entity model is not active, cannot close.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < MAX_NUM_SYSTEMS; i++) {
        if (systems_active[i] && systems[i].close != NULL) {
            systems[i].close(&systems[i]);
        }
    }

    // --- destroy all entities
    // --- free linked lists
    ENTITY_MODEL_ACTIVE = false;
}


#if 0
void destroy_entity(EntityID entity_id)
{
    EntityNode *cur = entity_nodes;
    do {
        if (cur->id == entity_id) {
            if (cur->marked_for_destruction) {
                // entity wasn't "destroyed" by this call, as it was already marked for destruction.
                return false;
            }
            cur->marked_for_destruction = true;
            return true;
        }
        cur = cur->next;
    } while (cur != NULL);
    return false;
}
/*
 * Destroys the entity and its subtree.
 */
static void annihilate_entity(Entity *entity)
{
    for (int i = 0; i < MAX_ENTITY_CHILDREN; i++) {
        if (entity->children[i] != NULL_ENTITY_ID) {
            Entity *child = get_entity(entity->children[i]);
            if (child != NULL) {
                annihilate_entity(child);
            }
        }
    }
    for (int i = 0; i < MAX_ENTITY_COMPONENTS; i++) {
        if (entity->components[i] != NULL_COMPONENT_ID) {
            Component *component = get_component(entity->components[i]);
            if (component != NULL) {
                annihilate_component(component);
            }
        }
    }
    free_entity(entity);
}
static void free_entity(Entity *entity)
{
    free(entity);
}

void destroy_component(ComponentID component_id)
{
    ComponentNode *cur = component_nodes;
    do {
        if (cur->id == component_id) {
            if (cur->marked_for_destruction) {
                // component wasn't "destroyed" by this call, as it was already marked for destruction.
                return false;
            }
            cur->marked_for_destruction = true;
            return true;
        }
        cur = cur->next;
    } while (cur != NULL);
    return false;
}

static void annihilate_component(Component *component)
{
    free_component(component);
}

static void free_component(Component *component)
{
    free(component);
}
#endif


void _iterator_components_of_type(ComponentType component_type, Iterator *iterator)
{
    /* printf("Making an iterator of type %d ...\n", component_type); */
    init_iterator(iterator, _iterator_components_of_type2);
    iterator->data1.int_val = component_type;
}
void _iterator_components_of_type2(Iterator *iterator)
{
    ComponentType component_type = (ComponentType) iterator->data1.int_val;
    ComponentNode *cur = (ComponentNode *) iterator->data2.ptr_val;
    if (cur != NULL) {
        printf("%d iterating at %ld ...\n", component_type, cur->component.entity_id);
    }
BEGIN_COROUTINE(iterator)
coroutine_start:
    cur = component_nodes;
coroutine_a:
    do {
        if (cur->component.type == component_type) {
            iterator->val = (void *) &cur->component;
            iterator->coroutine_flag = COROUTINE_A;
            iterator->data2.ptr_val = (void *) cur->next;
            return;
        }
        cur = cur->next;
    } while (cur != NULL);
    iterator->coroutine_flag = COROUTINE_B;
coroutine_b:
    iterator->val = NULL;
    return;
// ...
coroutine_c:
    return;
coroutine_d:
    return;
coroutine_e:
    return;
}


Component *get_entity_component_of_type_from_type_id(EntityID entity_id, ComponentType component_type)
{
    Entity *entity = get_entity(entity_id);

    // Returns the first found component of this type.
    for (int i = 0; i < MAX_ENTITY_COMPONENTS; i++) {
        if (entity->components[i] != NULL_COMPONENT_ID) {
            Component *component = get_component(entity->components[i]);
            if (component->type == component_type) {
                return component;
            }
        }
    }
    fprintf(stderr, "ERROR: Entity %s does not have a component of type ID %d.\n", entity->name, component_type);
    exit(EXIT_FAILURE);
}
