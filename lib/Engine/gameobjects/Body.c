/*================================================================================
    Body
================================================================================*/
#include "Engine.h"

AspectType Body_TYPE_ID;
void Body_init(Body *body, char *material_path, char *mesh_path)
{
    body->visible = true;
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

mat4x4 Body_matrix(Body *body)
{
    mat4x4 matrix = Transform_matrix(other_aspect(body, Transform));
    //---This body rescaling is a hack. This returns the same thing as used in the render loop.
    for (int i = 0; i < 3; i++) {
	for (int j = 0; j < 3; j++) {
	    matrix.vals[4*i + j] *= body->scale;
	}
    }
    return matrix;
}

