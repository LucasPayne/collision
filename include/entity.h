/*================================================================================
    Entity module.
================================================================================*/
#ifndef HEADER_DEFINED_ENTITY
#define HEADER_DEFINED_ENTITY
#include <stdint.h>
#include "iterator.h"


typedef uint16_t MapIndex;
typedef uint64_t UUID; // ?
#define NULL_ENTITY_ID { 0, 0 } // ???
typedef struct EntityID_s {
    MapIndex map_index;
    UUID uuid;
} EntityID;


typedef uint16_t AspectType;
// "null" overall is aspect type of 0 (is this horrible?)
#define NULL_ASPECT_TYPE 0
typedef struct AspectID_s {
    MapIndex map_index;
    UUID uuid;
    AspectType type;
} AspectID;


// "null" overall is null aspects entry (is this horrible?)
typedef struct EntityMapEntry_s {
    UUID uuid;
    uint16_t num_aspects;
    AspectID *aspects;
} EntityMapEntry;


// null manager: type 0
typedef struct Manager_s {
    AspectType type;
    uint16_t aspect_map_size;
    void **aspect_map;
    UUID last_uuid;
    void (*new_aspect)( struct Manager_s *, AspectID );
    void (*destroy_aspect) ( struct Manager_s *, AspectID );
    void (*aspect_iterator) ( Iterator * );
    void (*serialize) (FILE *, void *);
} Manager;


// in-line substruct macro (so, make sure to put this at the top of aspect structures, means they all get this metadata)
#define ASPECT_PROPERTIES()\
    AspectID aspect_id;\
    EntityID entity_id;
typedef struct AspectProperties_s {
    AspectID aspect_id;
    EntityID entity_id;
} AspectProperties;

EntityID new_entity(int start_num_aspects);

void *get_aspect_data(AspectID aspect);

#define entity_add_aspect(ENTITY_ID,ASPECT_TYPE_NAME)\
    (ASPECT_TYPE_NAME *) get_aspect_data(_entity_add_aspect(( ENTITY_ID ), ASPECT_TYPE_NAME ## _TYPE_ID))
AspectID _entity_add_aspect(EntityID entity, AspectType type);
void init_entity_model(void);

// A manager is a fancy (?) container. These defaults can do anything easy (such as malloc and iterate by going over the non-nulls in the map).
void default_manager_new_aspect(Manager *manager, AspectID aspect);
void default_manager_destroy_aspect(Manager *manager, AspectID aspect);
void default_manager_aspect_iterator(Iterator *iterator);

#define new_default_manager(ASPECT_TYPE_NAME,SERIALIZE)\
    _new_manager(ASPECT_TYPE_NAME ## _TYPE_ID, default_manager_new_aspect, default_manager_destroy_aspect, default_manager_aspect_iterator, ( SERIALIZE ))
#define new_manager(ASPECT_TYPE_NAME,NEW_ASPECT,DESTROY_ASPECT,ASPECT_ITERATOR,SERIALIZE)\
    _new_manager(ASPECT_TYPE_NAME ## _TYPE_ID, ( NEW_ASPECT ), ( DESTROY_ASPECT ), ( ASPECT_ITERATOR ), ( SERIALIZE ))
Manager *_new_manager(AspectType type,
                      void (*new_aspect)(Manager *, AspectID),
                      void (*destroy_aspect)(Manager *, AspectID),
                      void (*aspect_iterator)(Iterator *),
                      void (*serialize) (FILE *, void *));

Manager *manager_of_type(AspectType type);

//================================================================================
// Querying
//================================================================================
// Get data from aspect type
#define get_aspect_type(ENTITY_ID,ASPECT_TYPE_NAME)\
    (ASPECT_TYPE_NAME *) _get_aspect_type(( ENTITY_ID ), ASPECT_TYPE_NAME ## _TYPE_ID)
void *_get_aspect_type(EntityID entity, AspectType type);

/* Macro'd syntax for using manager's iterators (managers are really containers)
 * for_aspect(SeeingMesh, seeing_mesh)
        SeeingMesh *seeing_mesh = get_aspect_type(entity, SeeingMesh);
 *      Transform *transform = get_aspect_type(seeing_mesh->entity_id, Transform);
 *      if (transform->matrix.vals[0 + 3*4] > 2.0) seeing_mesh->visible = false;
 *      else seeing_mesh->visible = true;
 * end_for_aspect()
 *
 * Don't know how to make syntax better/braced. Functionizing this is possible without macros, passing a function
 * you want to feed pointers to, but can't be done in in-line code (?).
 */
#define for_aspect(ASPECT_TYPE_NAME,LVALUE)\
    {\
    ASPECT_TYPE_NAME *LVALUE;\
    Iterator iterator;\
    Manager *manager = manager_of_type(ASPECT_TYPE_NAME ## _TYPE_ID);\
    init_iterator(&iterator, manager->aspect_iterator);\
    iterator.data1.ptr_val = manager;\
    iterator.data2.int_val = 0;\
    while (1) {\
        step(&iterator);\
        if (iterator.val == NULL) {\
            break;\
        }\
        LVALUE = (ASPECT_TYPE_NAME *) iterator.val;
#define end_for_aspect()\
    }\
    }

//================================================================================
// purely printing functions
//================================================================================
void print_entities(void);
void print_entity(EntityID entity);

#define print_aspects_of_type(ASPECT_TYPE_NAME)\
    _print_aspects_of_type(ASPECT_TYPE_NAME ## _TYPE_ID)
void _print_aspects_of_type(AspectType type);



#endif // HEADER_DEFINED_ENTITY
