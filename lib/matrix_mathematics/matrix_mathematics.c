/*--------------------------------------------------------------------------------
   Definitions for the matrix mathematics module.
   See header for details.
--------------------------------------------------------------------------------*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "matrix_mathematics.h"
#include "helper_definitions.h"

// Static helper functions
//--------------------------------------------------------------------------------
static void fprint_matrixNxNf(FILE *file, Matrix4x4f *matrix, int N);
//--------------------------------------------------------------------------------

void right_multiply_matrix4x4f(Matrix4x4f *matrix, Matrix4x4f *B)
{
    float scratch[4 * 4];
    for (int i = 0; i < 4; i++) { // col of B
        for(int j = 0; j < 4; j++) { // row of matrix
            float dot  = 0;
            for (int k = 0; k < 4; k++) { // through the col of B and row of matrix
                dot += matrix->vals[j + 4*k] * B->vals[k + 4*i];
            }
            scratch[j + 4*i] = dot;
        }
    }
    memcpy(matrix->vals, scratch, sizeof(matrix->vals));
}
void right_multiply_by_transpose_matrix4x4f(Matrix4x4f *matrix, Matrix4x4f *B)
{
    float scratch[4 * 4];
    for (int i = 0; i < 4; i++) { // col of B
        for(int j = 0; j < 4; j++) { // row of matrix
            float dot  = 0;
            for (int k = 0; k < 4; k++) { // through the col of B and row of matrix
                dot += matrix->vals[j + 4*k] * B->vals[i + 4*k];
            }
            scratch[j + 4*i] = dot;
        }
    }
    memcpy(matrix->vals, scratch, sizeof(matrix->vals));
}

void identity_matrix3x3f(Matrix3x3f *matrix)
{
    memset(&matrix->vals, 0, sizeof(matrix->vals));
    for (int i = 0; i < 3; i++)
        matrix->vals[i + 3*i] = 1;
}
void right_multiply_matrix3x3f(Matrix3x3f *matrix, Matrix3x3f *B)
{
    Matrix3x3f scratch;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            float dot = 0;
            for (int k = 0; k < 3; k++) {
                dot += matrix->vals[j + 3*k] * B->vals[k + 3*i];
            }
            scratch.vals[j + 3*i] = dot;
        }
    }
    memcpy(&scratch.vals, &matrix->vals, sizeof(matrix->vals));
}

void print_matrix3x3f(Matrix3x3f *matrix)
{
    fprint_matrixNxNf(stdout, matrix, 3);
}
void print_matrix4x4f(Matrix4x4f *matrix)
{
    fprint_matrixNxNf(stdout, matrix, 4);
}
void fprint_matrix3x3f(FILE *file, Matrix3x3f *matrix)
{
    fprint_matrixNxNf(file, matrix, 3);
}
void fprint_matrix4x4f(FILE *file, Matrix4x4f *matrix)
{
    fprint_matrixNxNf(file, matrix, 4);
}
static void fprint_matrixNxNf(FILE *file, Matrix4x4f *matrix, int N)
{
    for (int i = 0; i < N; i++) {
        putchar('[');
        for (int j = 0; j < N; j++) {
            fprintf(file, "%.2lf", matrix->vals[i + N*j]);
            if (j != N - 1) fprintf(file, ", ");
        }
        fprintf(file, "]\n");
    }
}

void identity_matrix4x4f(Matrix4x4f *matrix)
{
    memset(matrix->vals, 0, sizeof(matrix->vals));
    matrix->vals[4*0 + 0] = 1.0;
    matrix->vals[4*1 + 1] = 1.0;
    matrix->vals[4*2 + 2] = 1.0;
    matrix->vals[4*3 + 3] = 1.0;
}

void x_angle_rotation_matrix4x4f(Matrix4x4f *matrix, float theta)
{
    identity_matrix4x4f(matrix);
    float sin_theta = sin(theta);
    float cos_theta = cos(theta);
    matrix->vals[1 + 4*1] = cos_theta;
    matrix->vals[2 + 4*1] = sin_theta;
    matrix->vals[1 + 4*2] = -sin_theta;
    matrix->vals[2 + 4*2] = cos_theta;
}
void y_angle_rotation_matrix4x4f(Matrix4x4f *matrix, float theta)
{
    identity_matrix4x4f(matrix);
    float sin_theta = sin(theta);
    float cos_theta = cos(theta);
    matrix->vals[0 + 4*0] = cos_theta;
    matrix->vals[2 + 4*0] = sin_theta;
    matrix->vals[0 + 4*2] = -sin_theta;
    matrix->vals[2 + 4*2] = cos_theta;
}
void z_angle_rotation_matrix4x4f(Matrix4x4f *matrix, float theta)
{
    identity_matrix4x4f(matrix);
    float sin_theta = sin(theta);
    float cos_theta = cos(theta);
    matrix->vals[0 + 4*0] = cos_theta;
    matrix->vals[1 + 4*0] = sin_theta;
    matrix->vals[0 + 4*1] = -sin_theta;
    matrix->vals[1 + 4*1] = cos_theta;
}


void euler_rotation_matrix3x3f(Matrix3x3f *matrix, float theta_x, float theta_y, float theta_z)
{
    identity_matrix3x3f(matrix);
    x_angle_rotate_matrix3x3f(matrix, theta_x);
    y_angle_rotate_matrix3x3f(matrix, theta_y);
    z_angle_rotate_matrix3x3f(matrix, theta_z);
}
void x_angle_rotate_matrix3x3f(Matrix3x3f *matrix, float theta)
{
    float cos_theta = cos(theta);
    float sin_theta = sin(theta);
    matrix->vals[0 + 3*0] = cos_theta * matrix->vals[0 + 3*0] + sin_theta * matrix->vals[0 + 3*1];
    matrix->vals[1 + 3*0] = cos_theta * matrix->vals[1 + 3*0] + sin_theta * matrix->vals[1 + 3*1];
    matrix->vals[0 + 3*1] = -sin_theta * matrix->vals[0 + 3*0] + cos_theta * matrix->vals[0 + 3*1];
    matrix->vals[1 + 3*1] = -sin_theta * matrix->vals[1 + 3*0] + cos_theta * matrix->vals[1 + 3*1];
}
void y_angle_rotate_matrix3x3f(Matrix3x3f *matrix, float theta)
{
    float cos_theta = cos(theta);
    float sin_theta = sin(theta);
    matrix->vals[0 + 3*0] = cos_theta * matrix->vals[0 + 3*0] + sin_theta * matrix->vals[0 + 3*2];
    matrix->vals[2 + 3*0] = cos_theta * matrix->vals[2 + 3*0] + sin_theta * matrix->vals[2 + 3*2];
    matrix->vals[0 + 3*2] = -sin_theta * matrix->vals[0 + 3*0] + cos_theta * matrix->vals[0 + 3*2];
    matrix->vals[2 + 3*2] = -sin_theta * matrix->vals[2 + 3*0] + cos_theta * matrix->vals[2 + 3*2];
}
void z_angle_rotate_matrix3x3f(Matrix3x3f *matrix, float theta)
{
    float cos_theta = cos(theta);
    float sin_theta = sin(theta);
    matrix->vals[1 + 3*1] = cos_theta * matrix->vals[1 + 3*1] + sin_theta * matrix->vals[1 + 3*2];
    matrix->vals[2 + 3*1] = cos_theta * matrix->vals[2 + 3*1] + sin_theta * matrix->vals[2 + 3*2];
    matrix->vals[1 + 3*2] = -sin_theta * matrix->vals[1 + 3*1] + cos_theta * matrix->vals[1 + 3*2];
    matrix->vals[2 + 3*2] = -sin_theta * matrix->vals[2 + 3*1] + cos_theta * matrix->vals[2 + 3*2];
}


// Euler rotations as embedded in 4x4 homogeneous coordinate matrices
void euler_rotation_matrix4x4f(Matrix4x4f *matrix, float theta_x, float theta_y, float theta_z)
{
    Matrix4x4f x_matrix;
    x_angle_rotation_matrix4x4f(&x_matrix, theta_x);
    Matrix4x4f y_matrix;
    y_angle_rotation_matrix4x4f(&y_matrix, theta_y);
    Matrix4x4f z_matrix;
    z_angle_rotation_matrix4x4f(&z_matrix, theta_z);
    identity_matrix4x4f(matrix);
    right_multiply_matrix4x4f(matrix, &x_matrix);
    right_multiply_matrix4x4f(matrix, &y_matrix);
    right_multiply_matrix4x4f(matrix, &z_matrix);
}

void euler_rotate_matrix4x4f(Matrix4x4f *matrix, float theta_x, float theta_y, float theta_z)
{
    Matrix4x4f x_matrix;
    x_angle_rotation_matrix4x4f(&x_matrix, theta_x);
    Matrix4x4f y_matrix;
    y_angle_rotation_matrix4x4f(&y_matrix, theta_y);
    Matrix4x4f z_matrix;
    z_angle_rotation_matrix4x4f(&z_matrix, theta_z);
    right_multiply_matrix4x4f(matrix, &x_matrix);
    right_multiply_matrix4x4f(matrix, &y_matrix);
    right_multiply_matrix4x4f(matrix, &z_matrix);
}


// Compose homogeneous 4x4 matrix with a translation
// ---- is this only logically the same when the lower-right entry is 1.0?
void translate_matrix4x4f(Matrix4x4f *matrix, float x, float y, float z)
{
    matrix->vals[0 + 4*3] += x;
    matrix->vals[1 + 4*3] += y;
    matrix->vals[2 + 4*3] += z;
}

void copy_matrix4x4f(Matrix4x4f *new, Matrix4x4f *copy_from)
{
    memcpy(new->vals, copy_from->vals, sizeof(new->vals));
}

void multiply_matrix4x4f(Matrix4x4f *to, Matrix4x4f *A, Matrix4x4f *B)
{
    copy_matrix4x4f(to, A);
    right_multiply_matrix4x4f(to, B);
}


void translate_rotate_2d_matrix3x3f(Matrix3x3f *matrix, float x, float y, float theta)
{
    memset(&matrix->vals, 0, sizeof(matrix->vals));
    float cos_theta = cos(theta);
    float sin_theta = sin(theta);
    matrix->vals[0 + 3*0] = cos_theta;
    matrix->vals[1 + 3*0] = sin_theta;
    matrix->vals[0 + 3*1] = -sin_theta;
    matrix->vals[1 + 3*1] = cos_theta;
    matrix->vals[0 + 3*2] = x;
    matrix->vals[1 + 3*2] = y;
    matrix->vals[2 + 3*2] = 1.0;
}

void translate_rotate_3d_matrix4x4f(Matrix4x4f *matrix, float x, float y, float z, float x_theta, float y_theta, float z_theta)
{
    euler_rotation_matrix4x4f(matrix, x_theta, y_theta, z_theta);
    matrix->vals[0 + 3*4] = x;
    matrix->vals[1 + 3*4] = y;
    matrix->vals[2 + 3*4] = z;
}

vec4 matrix_vec4(Matrix4x4f *matrix, vec4 v)
{
    vec4 vp;
    for (int i = 0; i < 4; i++) {
        vp.vals[i] = v.vals[0] * matrix->vals[i + 4*0]
                   + v.vals[1] * matrix->vals[i + 4*1]
                   + v.vals[2] * matrix->vals[i + 4*2]
                   + v.vals[3] * matrix->vals[i + 4*3];
    }
    return vp;
}

float vec3_dot(vec3 a, vec3 b)
{
    return a.vals[0]*b.vals[0] + a.vals[1]*b.vals[1] + a.vals[2]*b.vals[2];
}
vec3 vec3_add(vec3 a, vec3 b)
{
    vec3 v;
    v.vals[0] = a.vals[0]+b.vals[0];
    v.vals[1] = a.vals[1]+b.vals[1];
    v.vals[2] = a.vals[2]+b.vals[2];
    return v;
}
vec3 vec3_mul(vec3 a, float x)
{
    vec3 v;
    v.vals[0] = a.vals[0]*x;
    v.vals[1] = a.vals[1]*x;
    v.vals[2] = a.vals[2]*x;
    return v;
}
vec3 vec3_sub(vec3 a, vec3 b)
{
    vec3 v;
    v.vals[0] = a.vals[0]-b.vals[0];
    v.vals[1] = a.vals[1]-b.vals[1];
    v.vals[2] = a.vals[2]-b.vals[2];
    return v;
}
vec3 vec3_neg(vec3 a)
{
    vec3 v;
    v.vals[0] = -a.vals[0];
    v.vals[1] = -a.vals[1];
    v.vals[2] = -a.vals[2];
    return v;
}


// mat4x4 multiply_mat4x4(mat4x4 a, mat4x4 b)
// {
// 
// }

// Matrices in this module are row-major (term?): consecutive rows in a column are contiguous as indices.

vec3 matrix_vec3(mat3x3 m, vec3 v)
{
    vec3 vp;
    for (int i = 0; i < 3; i++) {
        vp.vals[i] = v.vals[0] * m.vals[i + 3*0]
                   + v.vals[1] * m.vals[i + 3*1]
                   + v.vals[2] * m.vals[i + 3*2];
    }
    return vp;
}
mat3x3 transpose_mat3x3(mat3x3 m)
{
    mat3x3 mt;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            mt.vals[3*i + j] = m.vals[3*j + i];
        }
    }
    return mt;
}
mat4x4 transpose_mat4x4(mat4x4 m)
{
    mat4x4 mt;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            mt.vals[4*i + j] = m.vals[4*j + i];
        }
    }
    return mt;
}


// "Rigid" matrices are 4x4 matrices which are the composition of a translation and a rotation.
// Due to this restriction, it is for example easier to invert them, so they have separate routines.
mat3x3 rotation_part_rigid_mat4x4(mat4x4 m)
{
    mat3x3 rot_part;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            rot_part.vals[3*i + j] = m.vals[4*i + j];
        }
    }
    return rot_part;
}
vec3 translation_vector_rigid_mat4x4(mat4x4 m)
{
    return new_vec3(m.vals[0 + 4*3], m.vals[1 + 4*3], m.vals[2 + 4*3]);
}


mat4x4 invert_rigid_mat4x4(mat4x4 m)
{
    mat3x3 inv_rot_part = transpose_mat3x3(rotation_part_rigid_mat4x4(m));
    vec3 rel_pos = matrix_vec3(inv_rot_part, translation_vector_rigid_mat4x4(m));
    mat4x4 minv = { 0 };
    // Transpose the top left 3x3 block.
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            minv.vals[i + 4*j] = m.vals[j + 4*i];
        }
    }
    minv.vals[0 + 4*3] = -rel_pos.vals[0];
    minv.vals[1 + 4*3] = -rel_pos.vals[1];
    minv.vals[2 + 4*3] = -rel_pos.vals[2];
    minv.vals[3 + 4*3] = 1;
    return minv;
}

mat4x4 identity_mat4x4(void)
{
    static const mat4x4 i4 = {{
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1,
    }};
    return i4;
}
mat4x4 x_angle_rotation_mat4x4(float theta)
{
    mat4x4 matrix = identity_mat4x4();
    float sin_theta = sin(theta);
    float cos_theta = cos(theta);
    matrix.vals[1 + 4*1] = cos_theta;
    matrix.vals[2 + 4*1] = sin_theta;
    matrix.vals[1 + 4*2] = -sin_theta;
    matrix.vals[2 + 4*2] = cos_theta;
    return matrix;
}
mat4x4 y_angle_rotation_mat4x4(float theta)
{
    mat4x4 matrix = identity_mat4x4();
    float sin_theta = sin(theta);
    float cos_theta = cos(theta);
    matrix.vals[0 + 4*0] = cos_theta;
    matrix.vals[2 + 4*0] = sin_theta;
    matrix.vals[0 + 4*2] = -sin_theta;
    matrix.vals[2 + 4*2] = cos_theta;
    return matrix;
}
mat4x4 z_angle_rotation_mat4x4(float theta)
{
    mat4x4 matrix = identity_mat4x4();
    float sin_theta = sin(theta);
    float cos_theta = cos(theta);
    matrix.vals[0 + 4*0] = cos_theta;
    matrix.vals[1 + 4*0] = sin_theta;
    matrix.vals[0 + 4*1] = -sin_theta;
    matrix.vals[1 + 4*1] = cos_theta;
    return matrix;
}
mat4x4 axis_angle_rotation_mat4x4(vec4 axis, float theta)
{
    mat4x4 m = identity_mat4x4();
    // ? --- ?
    mat4x4 x_rot = x_angle_rotation_mat4x4(theta * axis.vals[0]);
    right_multiply_matrix4x4f(&m, &x_rot);
    mat4x4 y_rot = y_angle_rotation_mat4x4(theta * axis.vals[1]);
    right_multiply_matrix4x4f(&m, &y_rot);
    mat4x4 z_rot = z_angle_rotation_mat4x4(theta * axis.vals[2]);
    right_multiply_matrix4x4f(&m, &z_rot);
    return m;
}

mat4x4 euler_angles_mat4x4(float theta_x, float theta_y, float theta_z)
{
    mat4x4 m = identity_mat4x4();
    mat4x4 x_rot = x_angle_rotation_mat4x4(theta_x);
    right_multiply_matrix4x4f(&m, &x_rot);
    vec4 rel_y = matrix_vec4(&m, new_vec4(0,1,0,1)); // just extracting a column.
    mat4x4 rot_y = axis_angle_rotation_mat4x4(rel_y, theta_y);
    right_multiply_matrix4x4f(&m, &rot_y);
    vec4 rel_z = matrix_vec4(&m, new_vec4(0,0,1,1));
    mat4x4 rot_z = axis_angle_rotation_mat4x4(rel_y, theta_z);
    right_multiply_matrix4x4f(&m, &rot_z);
    return m;
}


// Use a 4x4 matrix (supposedly a rigid transformation matrix) and use it to transform a 3-vector without translations.
// This is for transformation of normals.
//////////////////////////////////////////////////////////////////////////////////
// this is wrong.
//////////////////////////////////////////////////////////////////////////////////
vec3 matrix4_vec3_normal(Matrix4x4f *matrix, vec3 v)
{
    vec3 vp;
    for (int i = 0; i < 3; i++) {
        vp.vals[i] = v.vals[0] * matrix->vals[i + 4*0]
                   + v.vals[1] * matrix->vals[i + 4*1]
                   + v.vals[2] * matrix->vals[i + 4*2];
    }
    return vp;
}

void print_vec4(vec4 v)
{
    printf("(%f, %f, %f, %f)\n", v.vals[0], v.vals[1], v.vals[2], v.vals[3]);
}
void print_vec3(vec3 v)
{
    printf("(%f, %f, %f)\n", v.vals[0], v.vals[1], v.vals[2]);
}

vec4 new_vec4(float x, float y, float z, float w)
{
    vec4 v;
    v.vals[0] = x;
    v.vals[1] = y;
    v.vals[2] = z;
    v.vals[3] = w;
    return v;
}

vec3 new_vec3(float x, float y, float z)
{
    vec3 v;
    v.vals[0] = x;
    v.vals[1] = y;
    v.vals[2] = z;
    return v;
}


// Cross product
vec3 vec3_cross(vec3 u, vec3 v)
{
    // u x v = [ u2 v3 - u3 v2 ]
    //         [ u3 v1 - u1 v3 ]
    //         [ u1 v2 - u2 v1 ]
    return new_vec3(u.vals[1]*v.vals[2] - u.vals[2]*v.vals[1],
                    u.vals[2]*v.vals[0] - u.vals[0]*v.vals[2],
                    u.vals[0]*v.vals[1] - u.vals[1]*v.vals[0]);
}


vec3 vec3_zero(void)
{
    static const vec3 zero = {{0,0,0}};
    return zero;
}
vec4 vec4_zero(void)
{
    static const vec4 zero = {{0,0,0,0}};
    return zero;
}

vec3 vec3_normalize(vec3 v)
{
    const float epsilon = 0.0001;
    float length = vec3_length(v);
    if (length < epsilon) return vec3_zero(); // just fail by returning 0.
    float inv_length = 1/length;
    return vec3_mul(v, inv_length);
}

float vec3_square_length(vec3 v)
{
    return vec3_dot(v, v);
}

float vec3_length(vec3 v)
{
    return sqrt(vec3_dot(v, v));
}

vec4 vec3_to_vec4(vec3 v)
{
    vec4 v4;
    memcpy(&v4, &v, sizeof(float)*3);
    v4.vals[3] = 1.0;
    return v4;
}
vec3 vec4_to_vec3(vec4 v)
{
    vec3 v3;
    memcpy(&v3, &v, sizeof(float)*3);
    return v3;
}

vec3 vec3_lerp(vec3 a, vec3 b, float t)
{
    return vec3_add(a, vec3_mul(vec3_sub(b, a), t));
}
vec3 triangle_blend(vec3 a, vec3 b, vec3 c, float r, float s, float t)
{
    return vec3_add(vec3_mul(a, r), vec3_add(vec3_mul(b, s),
            		 vec3_mul(c, t)));

}

float vec3_square_dist(vec3 a, vec3 b)
{
    vec3 dif = vec3_sub(b, a);
    return vec3_dot(dif, dif);
}
float vec3_dist(vec3 a, vec3 b)
{
    return sqrt(vec3_square_dist(a, b));
}

vec3 mat4x4_vec3(mat4x4 *matrix, vec3 v)
{
    return vec4_to_vec3(matrix_vec4(matrix, vec3_to_vec4(v)));
}


vec2 new_vec2(float x, float y)
{
    vec2 v;
    v.vals[0] = x; 
    v.vals[1] = y; 
    return v;
}

vec4 color_mul(vec4 color, float x)
{
    // multiply non-alpha.
    for (int i = 0; i < 3; i++) color.vals[i] = x * color.vals[i];
    return color;
}
vec4 color_fade(vec4 color, float x)
{
    // multiply alpha only.
    color.vals[3] *= x;
    return color;
}


mat3x3 multiply_mat3x3(mat3x3 A, mat3x3 B)
{
    mat3x3 m = {0};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                m.vals[3*i + j] += A.vals[3*k + j] * B.vals[3*i + k];
            }
        }
    }
    return m;
}

mat3x3 mat3x3_add(mat3x3 A, mat3x3 B)
{
    mat3x3 m;
    for (int i = 0; i < 9; i++) m.vals[i] = A.vals[i] + B.vals[i];
    return m;
}

void mat3x3_orthonormalize(mat3x3 *m)
{
    // Gram-Schmidt. Subtract component of second column along the first, then normalize. Ignore the third column and just use the cross product.
    // This assumes that the matrix being orthonormalized is drifted from a right-handed orientation matrix.
    
    // Normalize col 1.
    float inv_col1_len = 1.0 / sqrt(m->vals[0]*m->vals[0]+m->vals[3]*m->vals[3]+m->vals[6]*m->vals[6]);
    m->vals[0] *= inv_col1_len;
    m->vals[3] *= inv_col1_len;
    m->vals[6] *= inv_col1_len;
    // Make col 2 orthogonal to col 1.
    float d = m->vals[0]*m->vals[1] + m->vals[3+0]*m->vals[3+1] + m->vals[2*3+0]*m->vals[2*3+1];
    m->vals[1] -= d * m->vals[0];
    m->vals[4] -= d * m->vals[3];
    m->vals[7] -= d * m->vals[6];
    // Normalize col 2.
    float inv_col2_len = 1.0 / sqrt(m->vals[1]*m->vals[1]+m->vals[4]*m->vals[4]+m->vals[7]*m->vals[7]);
    m->vals[1] *= inv_col2_len;
    m->vals[4] *= inv_col2_len;
    m->vals[7] *= inv_col2_len;
    // Infer col 3.
    vec3 c = vec3_cross(new_vec3(m->vals[0], m->vals[3], m->vals[6]), new_vec3(m->vals[1], m->vals[4], m->vals[7]));
    m->vals[2] = c.vals[0];
    m->vals[5] = c.vals[1];
    m->vals[8] = c.vals[2];
}

bool vec3_equal(vec3 a, vec3 b)
{
    for (int i = 0; i < 3; i++) {
        if (a.vals[i] != b.vals[i]) return false;
    }
    return true;
}

vec3 rand_vec3(float r)
{
    return new_vec3(frand()*r-r/2,frand()*r-r/2,frand()*r-r/2);
}
