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
// "null" is aspect type of 0
#define NULL_ASPECT_TYPE 0
typedef struct AspectID_s {
    MapIndex map_index;
    UUID uuid;
    AspectType type;
} AspectID;

// "null" is null aspects entry (is this horrible?)
typedef struct EntityMapEntry_s {
    UUID uuid;
    uint16_t num_aspects;
    AspectID *aspects;
    void *static_data;
} EntityMapEntry;


// null manager: type 0
typedef struct Manager_s {
    AspectType type;
    uint16_t aspect_map_size;
    void **aspect_map;
    UUID last_uuid;
    void (*new_aspect)( AspectID );
    void (*destroy_aspect) ( AspectID );
    void (*aspect_iterator) ( Iterator * );
} Manager;


typedef struct AspectProperties_s {
    AspectID aspect_id;
    EntityID entity_id;
} AspectProperties;

AspectID entity_add_aspect(EntityID entity, AspectType type);
void init_entity_model(void);

#endif // HEADER_DEFINED_ENTITY
