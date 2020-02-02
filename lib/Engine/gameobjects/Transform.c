/*================================================================================
    Transform aspect
================================================================================*/
#include "Engine.h"

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
vec3 Transform_position(Transform *t)
{
    return new_vec3(t->x, t->y, t->z);
}
vec3 Transform_angles(Transform *t)
{
    return new_vec3(t->theta_x, t->theta_y, t->theta_z);
}
Matrix4x4f Transform_matrix(Transform *transform)
{
    Matrix4x4f mat;
    translate_rotate_3d_matrix4x4f(&mat, transform->x, transform->y, transform->z, transform->theta_x, transform->theta_y, transform->theta_z);
    return mat;
}
vec3 Transform_relative_direction(Transform *t, vec3 direction)
{
    // Transform a direction vector.
    return matrix_vec3(rotation_part_rigid_mat4x4(Transform_matrix(t)), direction);
}
vec3 Transform_relative_position(Transform *t, vec3 position)
{
    // Transform a point from model space to world space.
    mat4x4 m = Transform_matrix(t);
    return vec4_to_vec3(matrix_vec4(&m, vec3_to_vec4(position)));
}
vec3 Transform_up(Transform *t)
{
    return Transform_relative_direction(t, new_vec3(0,1,0));
}
vec3 Transform_down(Transform *t)
{
    return Transform_relative_direction(t, new_vec3(0,-1,0));
}
vec3 Transform_left(Transform *t)
{
    return Transform_relative_direction(t, new_vec3(-1,0,0));
}
vec3 Transform_right(Transform *t)
{
    return Transform_relative_direction(t, new_vec3(1,0,0));
}
vec3 Transform_forward(Transform *t)
{
    return Transform_relative_direction(t, new_vec3(0,0,1));
}
vec3 Transform_backward(Transform *t)
{
    return Transform_relative_direction(t, new_vec3(0,0,-1));
}

void Transform_move(Transform *t, vec3 translation)
{
    //--idea: could make vectors structs with x,y,z[,w], and cast to an array when iteration is wanted.
    t->x += translation.vals[0];
    t->y += translation.vals[1];
    t->z += translation.vals[2];
}
void Transform_move_relative(Transform *t, vec3 translation)
{
    Transform_move(t, Transform_relative_direction(t, translation));
}
