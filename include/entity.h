/*
 * Entity component system.
 */

#ifndef HEADER_DEFINED_ENTITY
#define HEADER_DEFINED_ENTITY

#define MAX_NUM_SYSTEMS 16

#define MAX_SYSTEM_NAME_LENGTH 64
#define MAX_COMPONENT_NAME_LENGTH 32
#define MAX_ENTITY_NAME_LENGTH 32
#define MAX_ENTITY_CHILDREN 32
#define MAX_ENTITY_COMPONENTS 16

#define NULL_ENTITY_ID 0
#define UNIVERSE_ID 1 // 0 is reserved
#define NULL_COMPONENT_ID 0

#include "iterator.h"

typedef long unsigned int ComponentID;
typedef long unsigned int EntityID;
typedef signed int ComponentType;

typedef struct System_structure {
    char name[MAX_SYSTEM_NAME_LENGTH];
    void (*init) (struct System_structure *);
    void (*update) (struct System_structure *);
    void (*close) (struct System_structure *);
} System;
typedef struct Component_structure {
    ComponentID id;
    EntityID entity_id;
    ComponentType type;
    bool marked_for_destruction;
    char name[MAX_COMPONENT_NAME_LENGTH];
} Component;
typedef struct Entity_structure {
    EntityID id;
    EntityID parent_id;
    bool marked_for_destruction;
    char name[MAX_ENTITY_NAME_LENGTH];
    EntityID children[MAX_ENTITY_CHILDREN];
    ComponentID components[MAX_ENTITY_COMPONENTS];
} Entity;

//================================================================================
// Interface to the entity system
//================================================================================

/* Systems. (no system ID system currently) */
//--------------------------------------------------------------------------------
System *add_system(char *name, void (*init) (System *), void (*update) (System *), void (*close) (System *));

/* Entity and component iteration. */
//--------------------------------------------------------------------------------
#define iterator_components_of_type(COMPONENT_TYPE_NAME, ITERATOR)\
    _iterator_components_of_type(COMPONENT_TYPE_NAME ## _TYPE_ID, ( ITERATOR ));
// These should not be used. How to make them static? The macro expands to these.
void _iterator_components_of_type(ComponentType component_type, Iterator *iterator);
void _iterator_components_of_type2(Iterator *iterator);

/* One entity model can be active at a time. Wrap its usage in calls to initialize and close it. */
//--------------------------------------------------------------------------------
void init_entity_model();
void update_entity_model();
void close_entity_model();

/* Entity creation and component attachment. These return IDs. */
//--------------------------------------------------------------------------------
EntityID create_entity(EntityID parent_id, char *name);
/* These are macros, to implement "inheritence" for component types. */
#define entity_add_component(ENTITY_ID, NAME, COMPONENT_TYPE_NAME)\
    (COMPONENT_TYPE_NAME *) entity_add_component_from_types(( ENTITY_ID ), ( NAME ), COMPONENT_TYPE_NAME ## _TYPE_ID, sizeof( COMPONENT_TYPE_NAME ))
#define entity_add_component_get(ENTITY_ID, NAME, COMPONENT_TYPE_NAME)\
    (COMPONENT_TYPE_NAME *) entity_add_component_from_types_get(( ENTITY_ID ), ( NAME ), COMPONENT_TYPE_NAME ## _TYPE_ID, sizeof( COMPONENT_TYPE_NAME ))
/* Is there a way to make this static? The macro expands, so needs to be able to reference this. But it should not be used. */
ComponentID entity_add_component_from_types(EntityID entity_id, char *name, ComponentType component_type, size_t component_size);
Component *entity_add_component_from_types_get(EntityID entity_id, char *name, ComponentType component_type, size_t component_size);

/* Entity and component destruction. */
//--------------------------------------------------------------------------------
/* Return value:
 *     true when the entity/component did exist
 *     false when it didn't (no errors)
 */
bool destroy_entity(EntityID entity_id);
bool destroy_component(ComponentID component_id);

/* Retrieve objects from handles. */
//--------------------------------------------------------------------------------
Entity *get_entity(EntityID entity_id);
Component *get_component(ComponentID component_id);
// Retrieve components from entities and types 
#define get_entity_component_of_type(ENTITY_ID,COMPONENT_TYPE)\
    (COMPONENT_TYPE *) get_entity_component_of_type_from_type_id(( ENTITY_ID ), COMPONENT_TYPE ## _TYPE_ID)
Component *get_entity_component_of_type_from_type_id(EntityID entity_id, ComponentType component_type);

/* Debug and serialization functions. */
//--------------------------------------------------------------------------------
void print_entity_list(void);
void print_entity_tree(void);

//================================================================================
// Static in-module helper functions
//================================================================================
static Entity *ptr_create_entity(EntityID parent_id, char *name);
static void _print_entity_tree(Entity *entity, int indent_level);
static void entity_add_child(Entity *entity, Entity *child);
static EntityID new_entity_id(void);
static ComponentID new_component_id(void);
static void update_system(System *system);

#endif // HEADER_DEFINED_ENTITY
