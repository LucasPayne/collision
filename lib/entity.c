/*
 * Entity (or gameobject) model.
 *
 *
 * todo:
 *  typedef function types
 *  heirarchical transform (update this as part of "bookkeeping" of entity system?)
 *
 * Never realloc entities, or everything will be incoherent and pointers untrustworthy. Between creation and destruction
 * there must be a consistent address.
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
    

static Entity2D universe; // root "/"
static bool entity_model_active = false;


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
    printf("%s", entity->name);
    if (entity->num_children == 0) {
        printf("\n");
        return;
    }
    printf("/\n");
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
    entity->relative_transform.position.x = 0;
    entity->relative_transform.position.y = 0;
    entity->transform.rotation = 0;
    entity->relative_transform.rotation = 0;
    entity->name[0] = '\0';
    entity->init = NULL;
    entity->update = NULL;
    
    entity->parent = NULL;
    entity->num_children = 0;
    entity->child_space = 0;
    entity->children = NULL;

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

// Quickly create entity, for filling parameters yourself or for testing.
Entity2D *create_empty_entity(Entity2D *parent, char *name)
{
#if DEBUG
    entity_model_check();
#endif
    return _create_entity(parent, name, NULL, NULL, 0, 0, 0);
}

Entity2D *_create_entity(Entity2D *parent,
                           char *name,
                           void (*init) (struct Entity2D_s *),
                           void (*update) (struct Entity2D_s *),
                           double position_x,
                           double position_y,
                           double rotation)
{
#if DEBUG
    entity_model_check();
#endif
    if (parent == NULL) {
        parent = &universe;
    }

    Entity2D *new_entity = (Entity2D *) malloc(sizeof(Entity2D));
    mem_check(new_entity);
    zero_init_entity(new_entity);
    new_entity->child_space = START_NUM_CHILDREN;

    Entity2D **new_entity_children = (Entity2D **) malloc(sizeof(Entity2D *) * new_entity->child_space);
    mem_check(new_entity_children);
    new_entity->children = new_entity_children;

    strncpy(new_entity->name, name, MAX_ENTITY_NAME_LENGTH);

    new_entity->init = init;
    new_entity->update = update;
    new_entity->transform.position.x = position_x;
    new_entity->transform.position.y = position_y;
    new_entity->transform.rotation = rotation;

    new_entity->data = NULL; 

    new_entity->parent = parent;
    entity_add_child(parent, new_entity);

    return new_entity;
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
    free(entity);
}


void destroy_entity(Entity2D *entity)
{
#if DEBUG
    entity_model_check();
    if (entity == NULL) {
        fprintf(stderr, "ERROR: Attempted to destroy a null entity.\n");
        exit(EXIT_FAILURE);
    }
#endif

    /*
     * Could make this two separate functions, because nulling parent's reference doesn't matter except at the top level.
     */
    if (entity == &universe) {
        fprintf(stderr, "ERROR: Attempted to destroy the universe. Please don't.\n");
        exit(EXIT_FAILURE);
    }
    
    for (int i = 0; i < entity->num_children; i++) {
        if (entity->children[i] != NULL) {
            destroy_entity(entity->children[i]);
        }
    }

    for (int i = 0; i < entity->parent->num_children; i++) {
        if (entity->children[i] != NULL) {
            if (entity->parent->children[i] == entity) {
                // Parents must deal with dead children. They also may want to know, so don't go and update the entity list. (?)
                entity->parent->children[i] = NULL;
            }
        }
    }

    free_entity(entity);
}

void init_entity_model(void)
{
    if (entity_model_active) {
        fprintf(stderr, "ERROR: Entity model is already initialized.\n");
        exit(EXIT_FAILURE);
    }
    entity_model_active = true;

    zero_init_entity(&universe);
    strncpy(universe.name, "universe", MAX_ENTITY_NAME_LENGTH);
    universe.parent = NULL;

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

/*
 * Updating logic:
 * Depth first traversal, children all update before the parent.
 */
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
    if (entity->update != NULL) {
        entity->update(entity);
    }
}



Point2f point2f_transform_to_entity(Point2f point, Entity2D *entity)
{
    Point2f transformed_point;
    transformed_point.x =   cos(entity->transform.rotation) * (entity->transform.position.x + point.x)
			  + sin(entity->transform.rotation) * (entity->transform.position.y + point.y);

    transformed_point.y = - sin(entity->transform.rotation) * (entity->transform.position.x + point.x)
			  + cos(entity->transform.rotation) * (entity->transform.position.y + point.y);

    return transformed_point;
}


#undef DEBUG
