/*--------------------------------------------------------------------------------
   Definitions for the matrix mathematics module.
--------------------------------------------------------------------------------*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "matrix_mathematics.h"

static void print_matrixNxNf(Matrix4x4f *matrix, int N);

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

void print_matrix3x3f(Matrix4x4f *matrix)
{
    print_matrixNxNf(matrix, 3);
}
void print_matrix4x4f(Matrix4x4f *matrix)
{
    print_matrixNxNf(matrix, 4);
}
static void print_matrixNxNf(Matrix4x4f *matrix, int N)
{
    for (int i = 0; i < N; i++) {
        putchar('[');
        for (int j = 0; j < N; j++) {
            printf("%.2lf", matrix->vals[i + N*j]);
            if (j != N - 1) printf(", ");
        }
        printf("]\n");
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
    x_angle_rotation_matrix4x4f(matrix, theta_x);
    Matrix4x4f y_matrix;
    y_angle_rotation_matrix4x4f(&y_matrix, theta_y);
    Matrix4x4f z_matrix;
    z_angle_rotation_matrix4x4f(&z_matrix, theta_z);
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
