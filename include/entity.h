/*================================================================================
   Entity component system.
   - Systems
   - Entities
   - Components
   A single entity model can be active. This maintains a heirarchy of entities,
   which by themselves only have a place in the tree and a name and ID.
   Components can be attached to entities, which have a name and ID and a
   component type, as well as data in an "inheriting" structure.
   Systems have names and can be added and removed, and their behaviour
   is defined by their init, update, and close functions (all optional).

   The entity model updates by running over systems and calling their update functions.
   These update functions should use an iterator to iterate over components
   of certain types to do with the system.

   --- impose "component type subscriptions" on systems? They can only iterate through
       these component types.
   --- variable "system multiplexing" (200x one system per second, 60x renderer, etc.)
   --- system IDs
   --- hide and unhide entities, components, and systems without destroying
   --- destruction (deferred to a single part of update)

================================================================================*/
#ifndef HEADER_DEFINED_ENTITY
#define HEADER_DEFINED_ENTITY
#include "iterator.h"

#define MAX_NUM_SYSTEMS 16

#define MAX_SYSTEM_NAME_LENGTH 64
#define MAX_COMPONENT_NAME_LENGTH 32
#define MAX_ENTITY_NAME_LENGTH 32
#define MAX_ENTITY_CHILDREN 32
#define MAX_ENTITY_COMPONENTS 16

#define NULL_ENTITY_ID 0
#define UNIVERSE_ID 1 // 0 is reserved
#define NULL_COMPONENT_ID 0


//################################################################################
// Structure and type definitions
//################################################################################
typedef long unsigned int ComponentID;
typedef long unsigned int EntityID;
typedef long unsigned int SystemID; // what about long long ?
typedef signed int ComponentType;

typedef struct Component_structure {
    ComponentID id;
    EntityID entity_id;
    ComponentType type;
    bool on;
    bool marked_for_destruction;
    char name[MAX_COMPONENT_NAME_LENGTH];
} Component;
typedef struct Entity_structure {
    EntityID id;
    EntityID parent_id;
    bool on;
    bool marked_for_destruction;
    char name[MAX_ENTITY_NAME_LENGTH];
    EntityID children[MAX_ENTITY_CHILDREN];
    ComponentID components[MAX_ENTITY_COMPONENTS];
} Entity;
typedef struct System_structure {
    SystemID id;
    bool on;
    char name[MAX_SYSTEM_NAME_LENGTH];
    ComponentType component_type;
    void (*update) (Component *);
} System;

//################################################################################
// Interface to the entity system
//################################################################################

//================================================================================
// Systems
//================================================================================
#define add_system(NAME,COMPONENT_TYPE,UPDATE)\
    add_system_from_type_id(( NAME ), ( COMPONENT_TYPE ## _TYPE_ID ), ( UPDATE ))
SystemID add_system_from_type_id(char *name, ComponentType component_type, void (*update) (Component *));

//================================================================================
// Entity and component iteration.
//================================================================================
#define iterator_components_of_type(COMPONENT_TYPE_NAME, ITERATOR)\
    iterator_components_of_type_id(COMPONENT_TYPE_NAME ## _TYPE_ID, ( ITERATOR ));
// These should not be used. How to make them static? The macro expands to these.
void iterator_components_of_type_id(ComponentType component_type, Iterator *iterator);
void _iterator_components_of_type2(Iterator *iterator);

//================================================================================
// Object enabling and disabling
//================================================================================
void enable_system(SystemID system_id);
void enable_entity(EntityID entity_id);
void enable_component(ComponentID component_id);
void disable_system(SystemID system_id);
void disable_entity(EntityID entity_id);
void disable_component(ComponentID component_id);
bool system_is_enabled(SystemID system_id);
bool entity_is_enabled(EntityID entity_id);
bool component_is_enabled(ComponentID component_id);

//================================================================================
// One entity model can be active at a time.
// Wrap its usage in calls to initialize and close it.
//================================================================================
void init_entity_model();
void update_entity_model();
void close_entity_model();

//================================================================================
// Entity creation and component attachment. These return IDs.
//================================================================================
EntityID create_entity(EntityID parent_id, char *name);
/* These are macros, to implement "inheritence" for component types. */
#define entity_add_component(ENTITY_ID, NAME, COMPONENT_TYPE_NAME)\
    (COMPONENT_TYPE_NAME *) entity_add_component_from_types(( ENTITY_ID ),\
                                                ( NAME ),\
                                                COMPONENT_TYPE_NAME ## _TYPE_ID,\
                                                sizeof( COMPONENT_TYPE_NAME ))
#define entity_add_component_get(ENTITY_ID, NAME, COMPONENT_TYPE_NAME)\
    (COMPONENT_TYPE_NAME *) entity_add_component_from_types_get(( ENTITY_ID ),\
                                                ( NAME ),\
                                                COMPONENT_TYPE_NAME ## _TYPE_ID,\
                                                sizeof( COMPONENT_TYPE_NAME ))
/* Is there a way to make this static? The macro expands, so needs to be able to
 * reference this. But it should not be used. */
ComponentID entity_add_component_from_types(EntityID entity_id, char *name,
                            ComponentType component_type, size_t component_size);
Component *entity_add_component_from_types_get(EntityID entity_id, char *name,
                            ComponentType component_type, size_t component_size);

//================================================================================
// Entity and component destruction.
//================================================================================
/* Return value:
 *     true when the entity/component did exist
 *     false when it didn't (no errors)
 */
bool destroy_entity(EntityID entity_id);
bool destroy_component(ComponentID component_id);

//================================================================================
// Retrieve objects from handles.
//================================================================================
System *get_system(SystemID system_id);
Entity *get_entity(EntityID entity_id);
Component *get_component(ComponentID component_id);
// Retrieve components from entities and types 
#define get_entity_component_of_type(ENTITY_ID,COMPONENT_TYPE)\
    (COMPONENT_TYPE *) get_entity_component_of_type_from_type_id(( ENTITY_ID ),\
                                                     COMPONENT_TYPE ## _TYPE_ID)
Component *get_entity_component_of_type_from_type_id(EntityID entity_id,
                                                    ComponentType component_type);

//================================================================================
// Debug and serialization functions.
//================================================================================
void print_entity_list(void);
void print_entity_tree(void);

//################################################################################
// Static in-module helper functions
//################################################################################
static Entity *ptr_create_entity(EntityID parent_id, char *name);
static void _print_entity_tree(Entity *entity, int indent_level);
static void entity_add_child(Entity *entity, Entity *child);
static EntityID new_entity_id(void);
static ComponentID new_component_id(void);
static SystemID new_system_id(void);
static void update_system(System *system);
static void annihilate_entity(Entity *entity);
static void annihilate_component(Component *component);

#endif // HEADER_DEFINED_ENTITY
