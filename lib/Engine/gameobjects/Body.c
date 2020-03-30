/*================================================================================
    Body
================================================================================*/
#include "Engine.h"

AspectType Body_TYPE_ID;
void Body_init(Body *body, char *material_path, char *mesh_path)
{
    body->visible = true;
    body->material = new_resource_handle(Material, material_path);
    body->geometry = new_resource_handle(Geometry, mesh_path);
}


// The radius of the body, accounting for scale, being the maximum distance of a vertex from the model origin.
// This is used for a simple bounding sphere.
float Body_radius(Body *body)
{
    Transform *t = other_aspect(body, Transform);
    return resource_data(Geometry, body->geometry)->radius * t->scale;
}

mat4x4 Body_matrix(Body *body)
{
    return Transform_matrix(other_aspect(body, Transform));
}

