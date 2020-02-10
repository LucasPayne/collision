/*================================================================================
    Body
================================================================================*/
#include "Engine.h"

AspectType Body_TYPE_ID;
void Body_init(Body *body, char *material_path, char *mesh_path)
{
    body->scale = 1;
    body->material = new_resource_handle(Material, material_path);
    body->geometry = new_resource_handle(Geometry, mesh_path);
}


// The radius of the body, accounting for scale, being the maximum distance of a vertex from the model origin.
// This is used for a simple bounding sphere.
float Body_radius(Body *body)
{
    return resource_data(Geometry, body->geometry)->radius * body->scale;
}
