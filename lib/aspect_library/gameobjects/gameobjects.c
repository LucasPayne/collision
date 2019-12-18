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
void Body_init(Body *body, char *material_path, char *mesh_path)
{
    body->scale = 1;
    body->material = new_resource_handle(Material, material_path);
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
void Camera_init(Camera *camera, float aspect_ratio, float near_half_width, float near, float far)
{
    float l,r,b,t,n,f;
    r = near_half_width;
    l = -r;
    t = aspect_ratio * r;
    b = -t;
    n = near;
    f = far;

    Matrix4x4f frustrum_matrix = {0};
    frustrum_matrix.vals[4*0 + 0] = (2*n)/(r-l);
    frustrum_matrix.vals[4*1 + 1] = (2*n)/(t-b);
    frustrum_matrix.vals[4*2 + 2] = -(f+n)/(f-n);
    frustrum_matrix.vals[4*0 + 2] = (r+l)/(r-l);
    frustrum_matrix.vals[4*1 + 2] = (t+b)/(t-b);
    frustrum_matrix.vals[4*2 + 3] = -(2*n*f)/(f-n);
    frustrum_matrix.vals[4*3 + 2] = -1;

    camera->plane_r = r;
    camera->plane_l = l;
    camera->plane_t = t;
    camera->plane_b = b;
    camera->plane_n = n;
    camera->plane_f = f;
    camera->projection_matrix = frustrum_matrix;
}

