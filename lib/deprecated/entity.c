/*-------------------------------------------------------------------------------- 
== DEPRECATED, still works for some test applications ============================

   Definitions for the entity model.
   See the header for details.

   Notes
   -----------------------------------------------------------------------------
   I think the interface is fine.
   Think about containers, data structures, interfaces.
   The linear linked list was a "temporary" implementation.
   Why use this? Then the entity heirarchy is actually logically inferred
   from IDs, so is nothing to do with the linked list.
   ID lookup is also horrible. It should be constant.

   Right now this system seems fine, but redo this some time:
       --- think about data structures
       --- better model than the universe entity/component stuff

--------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "helper_definitions.h"
#include "deprecated/entity.h"
#include "iterator.h"

//--------------------------------------------------------------------------------
// Static declarations
//--------------------------------------------------------------------------------
static Entity *ptr_create_entity(EntityID parent_id, char *name);
static void _print_entity_tree(Entity *entity, int indent_level);
static void entity_add_child(Entity *entity, Entity *child);
static EntityID new_entity_id(void);
static ComponentID new_component_id(void);
static SystemID new_system_id(void);
static void update_system(System *system);
static void annihilate_entity(Entity *entity);
static void annihilate_component(Component *component);

//--------------------------------------------------------------------------------
// Linked-list implementation of entity and component pool
//--------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------
// Global entity model state
//--------------------------------------------------------------------------------
static System systems[MAX_NUM_SYSTEMS];
static bool systems_active[MAX_NUM_SYSTEMS];
static bool ENTITY_MODEL_ACTIVE = false;
static Entity *universe;
static EntityID current_entity_id;
static ComponentID current_component_id;
static SystemID current_system_id;

//--------------------------------------------------------------------------------
// Systems
//--------------------------------------------------------------------------------
SystemID add_system_from_type_id(char *name, ComponentType component_type, void (*update) (Component *))
{
#define TRACING 1
#if TRACING
    printf("Adding system named \"%s\" ...\n", name);
#endif

    for (int i = 0; i < MAX_NUM_SYSTEMS; i++) {
        if (!systems_active[i]) {
            strncpy(systems[i].name, name, MAX_SYSTEM_NAME_LENGTH);
            systems[i].component_type = component_type;
            systems[i].update = update;
            systems[i].on = true;
            systems_active[i] = true;
            systems[i].id = new_system_id();
            return systems[i].id;
        }
    }
    fprintf(stderr, "ERROR: not enough space to add a new system.\n");
    exit(EXIT_FAILURE);
#undef TRACING
}

//--------------------------------------------------------------------------------
// Entity and component iteration. (only iterates enabled objects).
//--------------------------------------------------------------------------------
void iterator_components_of_type_id(ComponentType component_type, Iterator *iterator)
{
    /* printf("Making an iterator of type %d ...\n", component_type); */
    init_iterator(iterator, _iterator_components_of_type2);
    iterator->data1.int_val = component_type;
}
void _iterator_components_of_type2(Iterator *iterator)
{
#define TRACING 0
    ComponentType component_type = (ComponentType) iterator->data1.int_val;
    ComponentNode *cur = (ComponentNode *) iterator->data2.ptr_val;
    /* ComponentNode *cur = NULL; */
#if TRACING
    if (cur != NULL) {
        printf("%d iterating at %ld ...\n", component_type, cur->component.entity_id);
    }
#endif
BEGIN_COROUTINE(iterator)
coroutine_start:
    cur = component_nodes;
coroutine_a:
    do {
        if (component_is_enabled(cur->component.id) && cur->component.type == component_type) {
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
#undef TRACING
}

//================================================================================
// Entity model usage
//================================================================================
void init_entity_model(void)
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
    universe->on = true;
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
void update_entity_model(void)
{
    // Run systems
    for (int i = 0; i < MAX_NUM_SYSTEMS; i++) {
        if (systems_active[i]) {
            update_system(&systems[i]);
        }
    }
    // Get rid of the objects marked for destruction
    {
        ComponentNode *cur = component_nodes;
        do {
            if (cur->component.marked_for_destruction) {
                annihilate_component(&cur->component);
            }
            cur = cur->next;
        } while (cur != NULL);
    }
    {
        EntityNode *cur = entity_nodes;
        do {
            if (cur->entity.marked_for_destruction) {
                annihilate_entity(&cur->entity);
            }
            cur = cur->next;
        } while (cur != NULL);
    }
}
static void update_system(System *system)
{
    if (system->update != NULL) {
        Iterator iterator;
        iterator_components_of_type_id(system->component_type, &iterator);
        while (1) {
            step(&iterator);
            if (iterator.val == NULL) {
                break;
            }
            Component *component = (Component *) iterator.val;
            system->update(component);
            if (iterator.data2.ptr_val == NULL) { // it appears i may need to do this. this iterator thing is too complicated and unneccessary
                // whole point goes out the window
                break;
            }
        }
    }
}
void close_entity_model(void)
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

    // --- destroy all entities
    // --- free linked lists
    ENTITY_MODEL_ACTIVE = false;
}

//--------------------------------------------------------------------------------
// Debug and serialization functions.
//--------------------------------------------------------------------------------
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
            if (!component_is_enabled(component->id)) {
                printf("[X]");
            }
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

//--------------------------------------------------------------------------------
// Entity creation and component attachment.
//--------------------------------------------------------------------------------
ComponentID entity_add_component_from_types(EntityID entity_id, char *name, ComponentType component_type, size_t component_size)
{
    return entity_add_component_from_types_get(entity_id, name, component_type, component_size)->id;
}
Component *entity_add_component_from_types_get(EntityID entity_id, char *name, ComponentType component_type, size_t component_size)
{
#define TRACING 0
#if TRACING
    printf("Adding component %s to entity with ID %ld ...\n", name, entity_id);
#endif
    ComponentNode *new_component_node = (ComponentNode *) calloc(1, component_size + (sizeof(ComponentNode) - sizeof(Component)));
        // hmm ...
    mem_check(new_component_node);
    Component *new_component = &new_component_node->component;
    new_component->type = component_type;
    new_component->on = true;
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
#undef TRACING
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
    new_entity->on = true;
    new_entity->parent_id = parent_id;

    Entity *parent = get_entity(parent_id);
    entity_add_child(parent, new_entity);

    new_entity_node->next = NULL;
    last_entity_node->next = new_entity_node;
    last_entity_node = new_entity_node;

    return new_entity;
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

//--------------------------------------------------------------------------------
// Retrieve objects from handles.
//--------------------------------------------------------------------------------
System *get_system(SystemID system_id)
{
    for (int i = 0; i < MAX_NUM_SYSTEMS; i++) {
        if (systems_active[i]) {
            if (systems[i].id == system_id) {
                return &systems[i];
            }
        }
    }
    return NULL;
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
    return NULL_COMPONENT_ID;
    /* fprintf(stderr, "ERROR: Entity %s does not have a component of type ID %d.\n", entity->name, component_type); */
    /* exit(EXIT_FAILURE); */
}

//--------------------------------------------------------------------------------
// Entity and component destruction.
//--------------------------------------------------------------------------------
bool destroy_entity(EntityID entity_id)
{
    // Destroying the universe does not actually destroy the universe entity, but destroys all children (so all other entities).
    EntityNode *cur = entity_nodes;
    do {
        if (cur->entity.id == entity_id) {
            if (cur->entity.marked_for_destruction) {
                // entity wasn't "destroyed" by this call, as it was already marked for destruction.
                return false;
            }
            cur->entity.marked_for_destruction = true;
            disable_entity(cur->entity.id);
            return true;
        }
        cur = cur->next;
    } while (cur != NULL);
    return false;
}
bool destroy_component(ComponentID component_id)
{
    ComponentNode *cur = component_nodes;
    do {
        if (cur->component.id == component_id) {
            if (cur->component.marked_for_destruction) {
                // component wasn't "destroyed" by this call, as it was already marked for destruction.
                return false;
            }
            cur->component.marked_for_destruction = true;
            disable_component(cur->component.id);
            return true;
        }
        cur = cur->next;
    } while (cur != NULL);
    return false;
}
static void annihilate_entity(Entity *entity)
{
    // First annihilate the components, then recur on subentities.
    for (int i = 0; i < MAX_ENTITY_COMPONENTS; i++) {
        if (entity->components[i] != NULL_COMPONENT_ID) {
            Component *component = get_component(entity->components[i]);
            annihilate_component(component);
        }
    }
    for (int i = 0; i < MAX_ENTITY_CHILDREN; i++) {
        if (entity->children[i] != NULL_ENTITY_ID) {
            Entity *entity = get_entity(entity->children[i]);
            annihilate_entity(entity);
        }
    }
    // The universe cannot be annihilated, but doing so (by destroying it) will have the effect of wiping all other entities.
    if (entity->id != UNIVERSE_ID) {
        EntityNode *cur = entity_nodes;
        EntityNode *prev = entity_nodes;
        // Remove the entity from the linear linked list
        do {
            if (cur->entity.id == entity->id) {
                // Remove from beginning of linked list
                if (cur == entity_nodes) {
                    if (cur->next != NULL && cur->next->next != NULL) {
                        cur->next = cur->next->next;
                    }
                }
                // Remove from end of linked list
                else if (cur->next == NULL) {
                    prev->next = NULL;
                    last_entity_node = prev;
                }
                // Remove from the middle of linked list
                else {
                    prev->next = cur->next;
                }
            }
            prev = cur;
            cur = cur->next;
        } while (cur != NULL);
    }
    // nullify parent's ID reference to this entity
    if (entity->parent_id != NULL_ENTITY_ID) {
        Entity *parent = get_entity(entity->parent_id);
        for (int i = 0; i < MAX_ENTITY_CHILDREN; i++) {
            if (parent->children[i] == entity->id) {
                parent->children[i] = NULL_ENTITY_ID;
            }
        }
    }
    // ... freeing stuff
    /* free(entity); */
}
static void annihilate_component(Component *component)
{
    // ... freeing stuff
    /* free(component); */
}


//--------------------------------------------------------------------------------
// ID allocation
//--------------------------------------------------------------------------------
static EntityID new_entity_id()
{
    return current_entity_id ++;
}
static ComponentID new_component_id()
{
    return current_component_id ++;
}
static SystemID new_system_id(void)
{
    return current_system_id ++;
}

//--------------------------------------------------------------------------------
// Object enabling and disabling
//--------------------------------------------------------------------------------
void enable_system(SystemID system_id)
{
    System *system = get_system(system_id);
    system->on = true;
}
void enable_entity(EntityID entity_id)
{
    Entity *entity = get_entity(entity_id);
    entity->on = true;
}
void enable_component(ComponentID component_id)
{
    Component *component = get_component(component_id);
    component->on = true;
}
void disable_system(SystemID system_id)
{
    System *system = get_system(system_id);
    system->on = false;
}
void disable_entity(EntityID entity_id)
{
    Entity *entity = get_entity(entity_id);
    entity->on = false;
}
void disable_component(ComponentID component_id)
{
    Component *component = get_component(component_id);
    component->on = false;
}

// Enabledness filters objects _before_ being iterated for systems. So, systems should not
// work with disabled objects/see them at all, yet they are not destroyed.
// Systems have this for completeness, but the functionality just toggles them in the loop
// (which could be an iterator for systems).
bool system_is_enabled(SystemID system_id)
{
    System *system = get_system(system_id);
    return system->on;
}
bool entity_is_enabled(EntityID entity_id)
{
    Entity *entity = get_entity(entity_id);
    if (entity == NULL) {
        return false;
    }
    if (entity->parent_id == NULL_ENTITY_ID) {
        return entity->on;
    }
    return entity->on && entity_is_enabled(entity->parent_id);
}
bool component_is_enabled(ComponentID component_id)
{
    Component *component = get_component(component_id);
    if (component == NULL) {
        return false;
    }
    return component->on && entity_is_enabled(component->entity_id);
}
