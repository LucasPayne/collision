/*================================================================================
    Transform aspect
================================================================================*/
#include "Engine.h"

AspectType Transform_TYPE_ID;
//---This transform initialization does not have scale as a parameter.
void Transform_set(Transform *transform, float x, float y, float z, float theta_x, float theta_y, float theta_z)
{
    transform->x = x;
    transform->y = y;
    transform->z = z;

    transform->rotation_matrix = euler_rotation_mat3x3(theta_x, theta_y, theta_z);
    transform->theta_x = theta_x;
    transform->theta_y = theta_y;
    transform->theta_z = theta_z;

    transform->scale = 1;
}
vec3 Transform_position(Transform *t)
{
    return new_vec3(t->x, t->y, t->z);
}
vec3 Transform_angles(Transform *t)
{
    return new_vec3(t->theta_x, t->theta_y, t->theta_z);
}
mat4x4 Transform_matrix(Transform *transform)
{
    mat4x4 mat = {0};
    if (transform->euler_controlled) {
        //----Does not take into account the center.
        translate_rotate_3d_mat4x4(&mat, transform->x, transform->y, transform->z, transform->theta_x, transform->theta_y, transform->theta_z);
        return mat;
    }
    // Copy over the orientation matrix to the upper-left block.
    // print_matrix3x3f(&transform->rotation_matrix);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            mat.vals[4*i + j] = transform->rotation_matrix.vals[3*i + j];
        }
    }
    mat.vals[15] = 1;
    mat.vals[4*3+0] = transform->x;
    mat.vals[4*3+1] = transform->y;
    mat.vals[4*3+2] = transform->z;

    // Scaling and recentering adjustments.
    //----Do this matrix multiplication directly instead of using the multiply function, to make this faster.
    mat4x4 scale_matrix;
    fill_mat4x4_cmaj(scale_matrix, transform->scale,0,0,0,
                              0,transform->scale,0,0,
                              0,0,transform->scale,0,
                              0,0,0,1);
    right_multiply_mat4x4(&mat, &scale_matrix);
    mat4x4 off_center_matrix;
    float cx,cy,cz;
    cx = transform->center.vals[0];
    cy = transform->center.vals[1];
    cz = transform->center.vals[2];
    fill_mat4x4_cmaj(off_center_matrix, 1,0,0,0,
                                   0,1,0,0,
                                   0,0,1,0,
                                   -cx,-cy,-cz,1);
    right_multiply_mat4x4(&mat, &off_center_matrix);

    // print_matrix4x4f(&mat);
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
    return vec4_to_vec3(matrix_vec4(m, vec3_to_vec4(position)));
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

void Transform_draw_axes(Transform *t, float length, float width)
{
    vec3 pos = Transform_position(t);
    paint_line_cv(Canvas3D, pos, vec3_add(pos, vec3_mul(Transform_forward(t), length)), "b", width);
    paint_line_cv(Canvas3D, pos, vec3_add(pos, vec3_mul(Transform_up(t), length)), "g", width);
    paint_line_cv(Canvas3D, pos, vec3_add(pos, vec3_mul(Transform_right(t), length)), "r", width);
}

// This is a routine since being Euler-controlled means the rotation matrix is described implicitly.
//---want to remove Euler-controlledness anyway.
mat3x3 Transform_rotation_matrix(Transform *t)
{
    if (!t->euler_controlled) return t->rotation_matrix;
    mat4x4 matrix = Transform_matrix(t);
    mat3x3 rotation_matrix;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            rotation_matrix.vals[3*i + j] = matrix.vals[4*i + j];
        }
    }
    return rotation_matrix;
}

