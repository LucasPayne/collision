/*
 *
 */

#ifndef HEADER_DEFINED_ENTITY
#define HEADER_DEFINED_ENTITY

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "helper_definitions.h"
#include "shapes.h"
#include "entity.h"

#define ENTITY_ID_NULL 0
#define COMPONENT_ID_NULL 0
#define ENTITY_UNIVERSE 1
#define COMPONENT_ID_START 1

#define MAX_ENTITY_NAME_LENGTH 32
#define MAX_COMPONENT_NAME_LENGTH 32

#define UNIVERSE_START_NUM_CHILDREN 64 // initial malloc'd space for children entities
#define START_NUM_CHILDREN 4 // in general
#define START_NUM_COMPONENTS 4

#define get_entity_data(TO_NAME,ENTITY,ENTITY_TYPE)\
    struct ENTITY_TYPE ## _properties_s *TO_NAME = ( struct ENTITY_TYPE ## _properties_s *) ENTITY ->data;
    
#define init_entity_data(ENTITY,ENTITY_TYPE)\
    ENTITY ->data = malloc(sizeof(struct ENTITY_TYPE ## _properties_s));\
    mem_check(ENTITY ->data);

#define entity_check(ENTITY)\
    if (( ENTITY ) == NULL) {\
        fprintf(stderr, "ERROR: Attempted to use a NULL entity.\n");\
    }


typedef struct Transform2D_s {
    Point2f position;
    double rotation; //theta anti-clockwise from -> in radians
} Transform2D;

typedef int ComponentType;
typedef unsigned long int ComponentID;
typedef struct Component_s {
    ComponentID id;
    EntityID entity_id;

    bool marked_for_destruction;

    char name[MAX_COMPONENT_NAME_LENGTH];
    ComponentType type;
    void *data;
} Component;


typedef unsigned long int EntityID;
typedef struct Entity2D_s {
    EntityID id;
    char name[MAX_ENTITY_NAME_LENGTH];

    Transform2D transform;
    bool marked_for_destruction;

    EntityID parent_id;

    void (*init) (struct Entity2D_s *);

    int num_children;
    int child_space;
    EntityID *child_ids;


    int num_components;
    int component_space;
    ComponentID *component_ids;

    void *data;
} Entity2D;


void print_entity_tree(Entity2D *entity);
static void _print_entity_tree(Entity2D *entity, int indent_level);
void zero_init_entity(Entity2D *entity);
void entity_add_child(Entity2D *parent, Entity2D *child);
EntityID create_entity(EntityID parent_id,
                       char *name,
                       void (*init) (struct Entity2D_s *),
                       double position_x,
                       double position_y,
                       double rotation);
void free_entity(Entity2D *entity);
void destroy_entity(Entity2D *entity);
void init_entity_model();
void close_entity_model();

void update_entity_model(void);
static void _update_entity_model(Entity2D *entity);

static void annihilate_entity(Entity2D *entity);
void destroy_entity(Entity2D *entity);

// Components
ComponentID entity_add_component(EntityID entity_id, ComponentType component_type, char *name);
void init_component(Component *component, Entity2D *entity, ComponentType component_type);
void destroy_component(Entity2D *entity, Component *component);

Entity2D *get_entity(EntityID id);
Entity2D *get_sub_entity(EntityID id, Entity2D *entity);

ComponentID new_component_id();
EntityID new_entity_id();

#endif
