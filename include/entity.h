
#ifndef HEADER_DEFINED_ENTITY
#define HEADER_DEFINED_ENTITY

#define MAX_COMPONENT_NAME_LENGTH 32
#define MAX_ENTITY_NAME_LENGTH 32
#define MAX_ENTITY_CHILDREN 32
#define MAX_ENTITY_COMPONENTS 16

#define NULL_ENTITY_ID 0
#define UNIVERSE_ID 1 // 0 is reserved
#define NULL_COMPONENT_ID 0

typedef long unsigned int ComponentID;
typedef long unsigned int EntityID;

typedef signed int ComponentType;

typedef struct Component_structure {
    ComponentID id;
    EntityID entity_id;
    ComponentType type;
    char name[MAX_COMPONENT_NAME_LENGTH];
} Component;
typedef struct ComponentNode_structure {
    Component component;
    struct ComponentNode_struture *next;
} ComponentNode;

typedef struct Entity_structure {
    EntityID id;
    EntityID parent_id;
    char name[MAX_ENTITY_NAME_LENGTH];
    EntityID children[MAX_ENTITY_CHILDREN];
    ComponentID components[MAX_ENTITY_COMPONENTS];
} Entity;
typedef struct EntityNode_structure {
    Entity entity;
    struct EntityNode_structure *next;
} EntityNode;

static ComponentID _entity_add_component(EntityID entity_id, char *name, ComponentType component_type);
Entity *get_entity(EntityID entity_id);
Component *get_component(ComponentID component_id);
EntityID new_entity_id();
ComponentID new_component_id();
EntityID create_entity(EntityID parent_id, char *name);
static Entity *ptr_create_entity(EntityID parent_id, char *name);
void init_entity_model();
void close_entity_model();
void print_entity_tree(void);
static void _print_entity_tree(Entity *entity, int indent_level);
static void entity_add_child(Entity *entity, Entity *child);

#endif
