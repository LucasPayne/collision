/*--------------------------------------------------------------------------------
"Gameobject" aspects.
Transform : 3D position, orientation, stored with Euler angles.
Body: Viewable mesh aspect.
Logic: Per-frame update logic.
Camera: 

Currently, this is not really a "library". A useful "game object" system above
the entity and resource systems should probably only be made by editing this a lot
and then deciding what aspects are useful, then making it a proper library.
--------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "rendering.h"
#include "matrix_mathematics.h"
#include "resources.h"
#include "entity.h"
#include "aspect_library/gameobjects.h"

void init_aspects_gameobjects(void)
{
    new_default_manager(Transform, NULL);
    new_default_manager(Body, NULL);
    new_default_manager(Logic, NULL);
    new_default_manager(Camera, NULL);
}

/*================================================================================
    Transform
================================================================================*/
AspectType Transform_TYPE_ID;
void Transform_set(Transform *transform, float x, float y, float z, float theta_x, float theta_y, float theta_z)
{
    transform->x = x;
    transform->y = y;
    transform->z = z;
    transform->theta_x = theta_x;
    transform->theta_y = theta_y;
    transform->theta_z = theta_z;
}
Matrix4x4f Transform_matrix(Transform *transform)
{
    Matrix4x4f mat;
    translate_rotate_3d_matrix4x4f(&mat, transform->x, transform->y, transform->z, transform->theta_x, transform->theta_y, transform->theta_z);
    return mat;
}

/*================================================================================
    Body
================================================================================*/
AspectType Body_TYPE_ID;
void Body_init(Body *body, char *artist_path, char *mesh_path)
{
    body->artist = new_resource_handle(Artist, artist_path);
    body->mesh = new_resource_handle(Mesh, mesh_path);
}

/*================================================================================
    Logic
================================================================================*/
AspectType Logic_TYPE_ID;

/*================================================================================
    Camera
================================================================================*/
AspectType Camera_TYPE_ID;
void Camera_init(Camera *camera)
{
    identity_matrix4x4f(&camera->projection_matrix);
}

