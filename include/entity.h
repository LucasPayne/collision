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

#define MAX_ENTITY_NAME_LENGTH 32

#define UNIVERSE_START_NUM_CHILDREN 64 // initial malloc'd space for children entities
#define START_NUM_CHILDREN 4 // in general

#define entity_model_check()\
    if (!entity_model_active) {\
        fprintf(stderr, "ERROR: trying to use entity functions while the entity model is not initialized.\n");\
        exit(EXIT_FAILURE);\
    }

#define create_entity(PARENT,NAME,ENTITY_TYPE,POSITION_X,POSITION_Y,ROTATION)\
    _create_entity(( PARENT ),\
                   ( NAME ),\
                   ( ENTITY_TYPE ## _entity_init ),\
                   ( ENTITY_TYPE ## _entity_update ),\
                   ( POSITION_X ),\
                   ( POSITION_Y ),\
                   ( ROTATION ))

#define get_entity_data(TO_NAME,ENTITY,ENTITY_TYPE)\
    struct ENTITY_TYPE ## _properties_s *TO_NAME = ( struct ENTITY_TYPE ## _properties_s *) ENTITY ->data;
    
#define init_entity_data(ENTITY,ENTITY_TYPE)\
    ENTITY ->data = malloc(sizeof(struct ENTITY_TYPE ## _properties_s));\
    mem_check(ENTITY ->data);


typedef struct Transform2D_s {
    Point2f position;
    double rotation; //theta anti-clockwise from -> in radians
} Transform2D;

typedef struct Entity2D_s {
    char name[MAX_ENTITY_NAME_LENGTH];
    Transform2D transform;
    Transform2D relative_transform;
    void (*init) (struct Entity2D_s *);
    void (*update) (struct Entity2D_s *);

    struct Entity2D_s *parent;
    int num_children;
    int child_space;
    struct Entity2D_s **children;
    void *data;
} Entity2D;


void print_entity_tree(Entity2D *entity);
static void _print_entity_tree(Entity2D *entity, int indent_level);
void zero_init_entity(Entity2D *entity);
void entity_add_child(Entity2D *parent, Entity2D *child);
Entity2D *_create_entity(Entity2D *parent,
                           char *name,
                           void (*init) (struct Entity2D_s *),
                           void (*update) (struct Entity2D_s *),
                           double position_x,
                           double position_y,
                           double rotation);
void free_entity(Entity2D *entity);
void destroy_entity(Entity2D *entity);
void init_entity_model();
void close_entity_model();
Entity2D *create_empty_entity(Entity2D *parent, char *name);

void update_entity_model(void);
static void _update_entity_model(Entity2D *entity);


Point2f point2f_transform_to_entity(Point2f point, Entity2D *entity);

#endif
