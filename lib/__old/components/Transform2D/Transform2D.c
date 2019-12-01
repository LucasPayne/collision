/*--------------------------------------------------------------------------------
   Definitions for component Transform2D
--------------------------------------------------------------------------------*/
#include "components/Transform2D.h"

//--------------------------------------------------------------------------------
// Static function declarations
//--------------------------------------------------------------------------------
static void _get_global_transform(Transform2D *transform, Transform2D *to);

//--------------------------------------------------------------------------------
// Definitions
//--------------------------------------------------------------------------------
void get_global_transform(Transform2D *transform, Transform2D *to)
{
    to->x = 0;
    to->y = 0;
    to->theta = 0;
    to->scale_x = 1;
    to->scale_y = 1;
    _get_global_transform(transform, to);
}
static void _get_global_transform(Transform2D *transform, Transform2D *to)
{
    to->x += transform->x;
    to->y += transform->x;
    to->theta += transform->theta;
    to->scale_x *= transform->scale_x;
    to->scale_y *= transform->scale_y;

    Entity *entity = get_entity(transform->component.entity_id);
    Entity *parent = get_entity(entity->parent_id);
    Transform2D *parent_transform = get_entity_component_of_type(parent->id, Transform2D);
    if (parent_transform != NULL) {
        _get_global_transform(parent_transform, to);
    }
}
