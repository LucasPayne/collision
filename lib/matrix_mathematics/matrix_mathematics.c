/*--------------------------------------------------------------------------------
    Implementations for the mathematics module, containing matrix and vector
    routines.
--------------------------------------------------------------------------------*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "matrix_mathematics.h"
#include "helper_definitions.h"

// 2-vector routines.
vec2 new_vec2(float x, float y)
{
    vec2 v;
    X(v) = x;
    Y(v) = y;
    return v;
}

// 3-vector routines.
//--------------------------------------------------------------------------------
vec3 new_vec3(float x, float y, float z)
{
    vec3 v;
    X(v) = x;
    Y(v) = y;
    Z(v) = z;
    return v;
}
vec3 vec3_zero(void)
{
    static const vec3 zero = {{0,0,0}};
    return zero;
}
vec3 vec3_add(vec3 a, vec3 b)
{
    vec3 v;
    for (int i = 0; i < 3; i++) v.vals[i] = a.vals[i] + b.vals[i];
    return v;
}
vec3 vec3_mul(vec3 a, float x)
{
    vec3 v;
    for (int i = 0; i < 3; i++) v.vals[i] = a.vals[i]*x;
    return v;
}
vec3 vec3_sub(vec3 a, vec3 b)
{
    vec3 v;
    for (int i = 0; i < 3; i++) v.vals[i] = a.vals[i]-b.vals[i];
    return v;
}
vec3 vec3_neg(vec3 a)
{
    vec3 v;
    for (int i = 0; i < 3; i++) v.vals[i] = -a.vals[i];
    return v;
}
vec3 vec3_cross(vec3 u, vec3 v)
{
    // u x v = [ u2 v3 - u3 v2 ]
    //         [ u3 v1 - u1 v3 ]
    //         [ u1 v2 - u2 v1 ]
    return new_vec3(Y(u)*Z(v) - Z(u)*Y(v),
                    Z(u)*X(v) - X(u)*Z(v),
                    X(u)*Y(v) - Y(u)*X(v));
}
float vec3_length(vec3 v)
{
    return sqrt(vec3_dot(v, v));
}
float vec3_square_length(vec3 v)
{
    return vec3_dot(v, v);
}
vec3 vec3_normalize(vec3 v)
{
    const float epsilon = 0.0001;
    float length = vec3_length(v);
    if (length < epsilon) return vec3_zero(); // just fail by returning 0.
    float inv_length = 1/length;
    return vec3_mul(v, inv_length);
}
vec3 vec3_lerp(vec3 u, vec3 v, float t)
{
    return vec3_add(vec3_mul(u, t), vec3_mul(v, 1 - t));
}
// Interpolate a->b by u to get A, ap->bp by u to get B, then interpolate A->B by v.
vec3 vec3_bilerp(vec3 a, vec3 b, vec3 ap, vec3 bp, float u, float v)
{
    return vec3_lerp(vec3_lerp(a, b, u), vec3_lerp(ap, bp, u), v);
}

// Barycentric combination. Assumes r+s+t = 1.
vec3 triangle_blend(vec3 a, vec3 b, vec3 c, float r, float s, float t)
{
    return vec3_add(vec3_mul(a, r), vec3_add(vec3_mul(b, s),
            		 vec3_mul(c, t)));
}
vec3 rand_vec3(float size)
{
    return new_vec3(size*(frand()-0.5), size*(frand()-0.5), size*(frand()-0.5));
}

// 4-vectors routines.
//--------------------------------------------------------------------------------
vec4 new_vec4(float x, float y, float z, float w)
{
    vec4 v;
    X(v) = x;
    Y(v) = y;
    Z(v) = z;
    W(v) = w;
    return v;
}
vec4 vec4_add(vec4 a, vec4 b)
{
    vec4 v;
    for (int i = 0; i < 4; i++) v.vals[i] = a.vals[i] + b.vals[i];
    return v;
}
vec4 vec4_mul(vec4 a, float x)
{
    vec4 v;
    for (int i = 0; i < 4; i++) v.vals[i] = a.vals[i]*x;
    return v;
}
vec4 vec4_sub(vec4 a, vec4 b)
{
    vec4 v;
    for (int i = 0; i < 4; i++) v.vals[i] = a.vals[i]-b.vals[i];
    return v;
}
vec4 vec4_neg(vec4 a)
{
    vec4 v;
    for (int i = 0; i < 4; i++) v.vals[i] = -a.vals[i];
    return v;
}
vec4 vec4_zero(void)
{
    static const vec4 zero = {{0,0,0,0}};
    return zero;
}
vec4 vec4_lerp(vec4 u, vec4 v, float t)
{
    return vec4_add(u, vec4_mul(vec4_sub(v, u), t));
}
// Interpolate a->b by u to get A, ap->bp by u to get B, then interpolate A->B by v.
vec4 vec4_bilerp(vec4 a, vec4 b, vec4 ap, vec4 bp, float u, float v)
{
    return vec4_lerp(vec4_lerp(a, b, u), vec4_lerp(ap, bp, u), v);
}


// 3x3 matrix routines.
//--------------------------------------------------------------------------------
void fill_identity_mat3x3(mat3x3 *m)
{
    static const mat3x3 identity = {{
        1,0,0,
        0,1,0,
        0,0,1,
    }};
    *m = identity;
}
mat3x3 identity_mat3x3(void)
{
    static const mat3x3 identity = {{
        1,0,0,
        0,1,0,
        0,0,1,
    }};
    return identity;
}
float mat3x3_determinant(mat3x3 m)
{
    return   m.vals[0]*(m.vals[4]*m.vals[8]-m.vals[5]*m.vals[7])
           - m.vals[1]*(m.vals[3]*m.vals[8]-m.vals[5]*m.vals[6])
           + m.vals[2]*(m.vals[3]*m.vals[7]-m.vals[4]*m.vals[6]);
}


mat3x3 mat3x3_inverse(mat3x3 m)
{
    // Cramer's rule.
    mat3x3 minv;
    float det = mat3x3_determinant(m);
    float detinv = 1.0 / det;
    #define term(Ar,Ac,Br,Bc,Cr,Cc,Dr,Dc) ( m.vals[3*(Ar-1)+ Ac-1]*m.vals[3*(Dr-1)+ Dc-1] - m.vals[3*(Br-1)+ Bc-1]*m.vals[3*(Cr-1)+ Cc-1] )
    minv.vals[0] = term(2,2,2,3,3,2,3,3);
    minv.vals[1] = term(1,3,1,2,3,3,3,2);
    minv.vals[2] = term(1,2,1,3,2,2,2,3);
    minv.vals[3] = term(2,3,2,1,3,3,3,1);
    minv.vals[4] = term(1,1,1,3,3,1,3,3);
    minv.vals[5] = term(1,3,1,1,2,3,2,1);
    minv.vals[6] = term(2,1,2,2,3,1,3,2);
    minv.vals[7] = term(1,2,1,1,3,2,3,1);
    minv.vals[8] = term(1,1,1,2,2,1,2,2);
    for (int i = 0; i < 9; i++) minv.vals[i] *= detinv;
    return minv;
}
void right_multiply_mat3x3(mat3x3 *matrix, mat3x3 *B)
{
    // In-place multiplication of the left matrix by B.
    mat3x3 scratch;
    for (int i = 0; i < 3; i++) { // row of the matrix
        for (int j = 0; j < 3; j++) { // column of B
            float dot = 0;
            for (int k = 0; k < 3; k++) {
                dot += matrix->vals[3*k + i] * B->vals[3*j + k];
            }
            scratch.vals[3*j + i] = dot;
        }
    }
    memcpy(matrix->vals, scratch.vals, sizeof(matrix->vals));
}
mat3x3 mat3x3_multiply(mat3x3 A, mat3x3 B)
{
    right_multiply_mat3x3(&A, &B);
    return A;
}
mat3x3 mat3x3_multiply3(mat3x3 A, mat3x3 B, mat3x3 C)
{
    right_multiply_mat3x3(&A, &B);
    right_multiply_mat3x3(&A, &C);
    return A;
}
mat3x3 mat3x3_multiply4(mat3x3 A, mat3x3 B, mat3x3 C, mat3x3 D)
{
    right_multiply_mat3x3(&A, &B);
    right_multiply_mat3x3(&A, &C);
    right_multiply_mat3x3(&A, &D);
    return A;
}
mat3x3 mat3x3_transpose(mat3x3 m)
{
    mat3x3 mt;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            mt.vals[3*i + j] = m.vals[3*j + i];
        }
    }
    return mt;
}

mat3x3 x_axis_rotation_mat3x3(float theta)
{
    mat3x3 matrix;
    float c = cos(theta);
    float s = sin(theta);
    fill_mat3x3_rmaj(matrix, 1, 0, 0,
                             0, c, -s,
                             0, s, c);
    return matrix;
}
mat3x3 y_axis_rotation_mat3x3(float theta)
{
    mat3x3 matrix;
    float c = cos(theta);
    float s = sin(theta);
    fill_mat3x3_rmaj(matrix, c, 0, -s,
                             0, 1, 0,
                             s, 0, c);
    return matrix;
}
mat3x3 z_axis_rotation_mat3x3(float theta)
{
    mat3x3 matrix;
    float c = cos(theta);
    float s = sin(theta);
    fill_mat3x3_rmaj(matrix, c, -s, 0,
                             s,  c, 0,
                             0,  0, 1);
    return matrix;
}

mat3x3 euler_rotation_mat3x3(float theta_x, float theta_y, float theta_z)
{
    return mat3x3_multiply3(x_axis_rotation_mat3x3(theta_x),
                            y_axis_rotation_mat3x3(theta_y),
                            z_axis_rotation_mat3x3(theta_z));
}

mat3x3 axis_angle_rotate_mat3x3(mat3x3 matrix, vec3 axis, float theta)
{
    // The axis is expected to be of unit length.
    vec3 *cols = (vec3 *) matrix.vals; // matrices are column major, so this should work.
    float costheta = cos(theta);
    float sintheta = sin(theta);
    for (int i = 0; i < 3; i++) {
        vec3 along = vec3_mul(axis, vec3_dot(cols[i], axis));
        vec3 orthogonal = vec3_sub(cols[i], along);
        vec3 n = vec3_cross(orthogonal, axis);
        cols[i] = vec3_add(along, vec3_add(vec3_mul(orthogonal, costheta), vec3_mul(n, sintheta)));
    }
    return matrix;
}

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
mat3x3 mat3x3_add(mat3x3 A, mat3x3 B)
{
    for (int i = 0; i < 9; i++) A.vals[i] += B.vals[i];
    return A;
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
mat3x3 mat3x3_mul(mat3x3 m, float x)
{
    for (int i = 0; i < 9; i++) m.vals[i] *= x;
    return m;
}

// 4x4 matrix routines.
//--------------------------------------------------------------------------------
mat4x4 identity_mat4x4(void)
{
    static const mat4x4 identity = {{
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1,
    }};
    return identity;
}
void fill_identity_mat4x4(mat4x4 *m)
{
    static const mat4x4 identity = {{
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1,
    }};
    *m = identity;
}

void right_multiply_mat4x4(mat4x4 *matrix, mat4x4 *B)
{
    // In-place multiplication of the left matrix by B.
    float scratch[4 * 4];
    for (int i = 0; i < 4; i++) { // row of the matrix
        for(int j = 0; j < 4; j++) { // column of B
            float dot  = 0;
            for (int k = 0; k < 4; k++) {
                dot += matrix->vals[4*k + i] * B->vals[4*j + k];
            }
            scratch[4*j + i] = dot;
        }
    }
    memcpy(matrix->vals, scratch, sizeof(matrix->vals));
}
mat4x4 mat4x4_multiply(mat4x4 A, mat4x4 B)
{
    right_multiply_mat4x4(&A, &B);
    return A;
}
mat4x4 mat4x4_multiply3(mat4x4 A, mat4x4 B, mat4x4 C)
{
    right_multiply_mat4x4(&A, &B);
    right_multiply_mat4x4(&A, &C);
    return A;
}
mat4x4 mat4x4_multiply4(mat4x4 A, mat4x4 B, mat4x4 C, mat4x4 D)
{
    right_multiply_mat4x4(&A, &B);
    right_multiply_mat4x4(&A, &C);
    right_multiply_mat4x4(&A, &D);
    return A;
}
mat4x4 mat4x4_transpose(mat4x4 m)
{
    mat4x4 mt;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            mt.vals[4*i + j] = m.vals[4*j + i];
        }
    }
    return mt;
}
vec4 matrix_vec4(mat4x4 matrix, vec4 v)
{
    vec4 vp;
    for (int i = 0; i < 4; i++) {
        vp.vals[i] = v.vals[0] * matrix.vals[i + 4*0]
                   + v.vals[1] * matrix.vals[i + 4*1]
                   + v.vals[2] * matrix.vals[i + 4*2]
                   + v.vals[3] * matrix.vals[i + 4*3];
    }
    return vp;
}

float mat4x4_determinant(mat4x4 m)
{
    // - Computationally inefficient, since this repacks entries into 3x3 matrices.
    // printf("Taking determinant of\n");
    // print_mat4x4(m);
    
    float det = 0;
    for (int i = 0; i < 4; i++) {
        float coefficient = m.vals[i];
        // printf("Coefficient: %.2f\n", coefficient);
        mat3x3 submatrix;
        for (int j = 1; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                if (k == i) continue;
                int kk = k > i ? k - 1 : k;
                submatrix.vals[3*(j - 1) + kk] = m.vals[4*j + k];
            }
        }
        // print_mat3x3(submatrix);
        det += (i % 2 == 0 ? 1 : -1) * coefficient * mat3x3_determinant(submatrix);
    }
    return det;
}
vec4 mat4x4_solve(mat4x4 m, vec4 p)
{
    // Solve for v in equation Mv = p.
    // Uses Cramer's rule.
    vec4 solution;
    mat4x4 new_matrix = m;
    float det = mat4x4_determinant(m);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            new_matrix.vals[4*i + j] = p.vals[j];
        }
        // Put back the original column that the previous iteration replaced.
        if (i > 0) {
            for (int j = 0; j < 4; j++) {
                new_matrix.vals[4*(i-1) + j] = m.vals[4*(i-1) + j];
            }
        }
        float x = mat4x4_determinant(new_matrix);
        solution.vals[i] = x / det;
    }
    return solution;
}
mat4x4 mat4x4_inverse(mat4x4 m)
{
    mat4x4 inverse;
    //--- Very inefficient.
    for (int i = 0; i < 4; i++) {
        vec4 basis_vector = {0};
        basis_vector.vals[i] = 1;
        vec4 solution = mat4x4_solve(m, basis_vector);
        for (int j = 0; j < 4; j++) {
            inverse.vals[4*i + j] = solution.vals[j];
        }
    }
    return inverse;
}


mat4x4 mat4x4_lookat(vec3 origin, vec3 look_at, vec3 approx_up)
{
    vec3 toward = vec3_neg(vec3_normalize(vec3_sub(look_at, origin)));
    vec3 up = vec3_normalize(vec3_sub(approx_up, vec3_mul(toward, vec3_dot(approx_up, toward))));
    vec3 right = vec3_cross(up, toward);
    mat4x4 matrix;
    fill_mat4x4_cmaj(matrix, X(right), X(up), X(toward), X(origin),
                             Y(right), Y(up), Y(toward), Y(origin),
                             Z(right), Z(up), Z(toward), Z(origin),
                                    0,     0,         0,         1);
    return matrix;
}

// Act on a vector with adjoined w-coordinate of 1, then convert back to a vec3.
// No division involved. This method assumes a rigid matrix (a frame of reference).
vec3 rigid_matrix_vec3(mat4x4 matrix, vec3 v)
{
    vec4 vp = matrix_vec4(matrix, new_vec4(X(v), Y(v), Z(v), 1));
    return new_vec3(X(vp), Y(vp), Z(vp));
}

// Printing
//--------------------------------------------------------------------------------
void print_vec3(vec3 v)
{
    printf("(%.6f, %.6f, %.6f)\n", X(v), Y(v), Z(v));
}
void print_vec4(vec4 v)
{
    printf("(%.6f, %.6f, %.6f, %.6f)\n", X(v), Y(v), Z(v), W(v));
}
// Matrices are printed as normal, with columns vertical.
void print_mat3x3(mat3x3 m)
{
    for (int i = 0; i < 3; i++) {
        printf("[");
        for (int j = 0; j < 3; j++) {
            printf(j < 2 ? "%.6f, " : "%.6f", m.vals[3*j + i]);
        }
        printf("]\n");
    }
}
void print_mat4x4(mat4x4 m)
{
    for (int i = 0; i < 4; i++) {
        printf("[");
        for (int j = 0; j < 4; j++) {
            printf(j < 3 ? "%.6f, " : "%.6f", m.vals[4*j + i]);
        }
        printf("]\n");
    }
}

// Conversion routines.
vec3 vec4_to_vec3(vec4 v)
{
    return *((vec3 *) &v);
}
vec4 vec3_to_vec4(vec3 v)
{
    return new_vec4(X(v), Y(v), Z(v), 1);
}
vec3 mat4x4_vec3(mat4x4 m, vec3 v)
{
    return vec4_to_vec3(matrix_vec4(m, vec3_to_vec4(v)));
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
    mat3x3 inv_rot_part = mat3x3_transpose(rotation_part_rigid_mat4x4(m));
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

// Euler angles.
// The specific euler angles used are extrinsic,
// Global X axis, Global Y axis, Global Z axis.
void x_angle_rotation_mat4x4(mat4x4 *matrix, float theta)
{
    fill_identity_mat4x4(matrix);
    float sin_theta = sin(theta);
    float cos_theta = cos(theta);
    matrix->vals[1 + 4*1] = cos_theta;
    matrix->vals[2 + 4*1] = sin_theta;
    matrix->vals[1 + 4*2] = -sin_theta;
    matrix->vals[2 + 4*2] = cos_theta;
}
void y_angle_rotation_mat4x4(mat4x4 *matrix, float theta)
{
    fill_identity_mat4x4(matrix);
    float sin_theta = sin(theta);
    float cos_theta = cos(theta);
    matrix->vals[0 + 4*0] = cos_theta;
    matrix->vals[2 + 4*0] = sin_theta;
    matrix->vals[0 + 4*2] = -sin_theta;
    matrix->vals[2 + 4*2] = cos_theta;
}
void z_angle_rotation_mat4x4(mat4x4 *matrix, float theta)
{
    fill_identity_mat4x4(matrix);
    float sin_theta = sin(theta);
    float cos_theta = cos(theta);
    matrix->vals[0 + 4*0] = cos_theta;
    matrix->vals[1 + 4*0] = sin_theta;
    matrix->vals[0 + 4*1] = -sin_theta;
    matrix->vals[1 + 4*1] = cos_theta;
}
void x_angle_rotate_mat3x3(mat3x3 *matrix, float theta)
{
    float cos_theta = cos(theta);
    float sin_theta = sin(theta);
    matrix->vals[0 + 3*0] = cos_theta * matrix->vals[0 + 3*0] + sin_theta * matrix->vals[0 + 3*1];
    matrix->vals[1 + 3*0] = cos_theta * matrix->vals[1 + 3*0] + sin_theta * matrix->vals[1 + 3*1];
    matrix->vals[0 + 3*1] = -sin_theta * matrix->vals[0 + 3*0] + cos_theta * matrix->vals[0 + 3*1];
    matrix->vals[1 + 3*1] = -sin_theta * matrix->vals[1 + 3*0] + cos_theta * matrix->vals[1 + 3*1];
}
void y_angle_rotate_mat3x3(mat3x3 *matrix, float theta)
{
    float cos_theta = cos(theta);
    float sin_theta = sin(theta);
    matrix->vals[0 + 3*0] = cos_theta * matrix->vals[0 + 3*0] + sin_theta * matrix->vals[0 + 3*2];
    matrix->vals[2 + 3*0] = cos_theta * matrix->vals[2 + 3*0] + sin_theta * matrix->vals[2 + 3*2];
    matrix->vals[0 + 3*2] = -sin_theta * matrix->vals[0 + 3*0] + cos_theta * matrix->vals[0 + 3*2];
    matrix->vals[2 + 3*2] = -sin_theta * matrix->vals[2 + 3*0] + cos_theta * matrix->vals[2 + 3*2];
}
void z_angle_rotate_mat3x3(mat3x3 *matrix, float theta)
{
    float cos_theta = cos(theta);
    float sin_theta = sin(theta);
    matrix->vals[1 + 3*1] = cos_theta * matrix->vals[1 + 3*1] + sin_theta * matrix->vals[1 + 3*2];
    matrix->vals[2 + 3*1] = cos_theta * matrix->vals[2 + 3*1] + sin_theta * matrix->vals[2 + 3*2];
    matrix->vals[1 + 3*2] = -sin_theta * matrix->vals[1 + 3*1] + cos_theta * matrix->vals[1 + 3*2];
    matrix->vals[2 + 3*2] = -sin_theta * matrix->vals[2 + 3*1] + cos_theta * matrix->vals[2 + 3*2];
}
// Euler rotations as embedded in 4x4 homogeneous coordinate matrices
void euler_rotation_mat4x4(mat4x4 *matrix, float theta_x, float theta_y, float theta_z)
{
    mat4x4 x_matrix;
    x_angle_rotation_mat4x4(&x_matrix, theta_x);
    mat4x4 y_matrix;
    y_angle_rotation_mat4x4(&y_matrix, theta_y);
    mat4x4 z_matrix;
    z_angle_rotation_mat4x4(&z_matrix, theta_z);
    fill_identity_mat4x4(matrix);
    right_multiply_mat4x4(matrix, &x_matrix);
    right_multiply_mat4x4(matrix, &y_matrix);
    right_multiply_mat4x4(matrix, &z_matrix);
}
void euler_rotate_mat4x4(mat4x4 *matrix, float theta_x, float theta_y, float theta_z)
{
    mat4x4 x_matrix;
    x_angle_rotation_mat4x4(&x_matrix, theta_x);
    mat4x4 y_matrix;
    y_angle_rotation_mat4x4(&y_matrix, theta_y);
    mat4x4 z_matrix;
    z_angle_rotation_mat4x4(&z_matrix, theta_z);
    right_multiply_mat4x4(matrix, &x_matrix);
    right_multiply_mat4x4(matrix, &y_matrix);
    right_multiply_mat4x4(matrix, &z_matrix);
}
void translate_rotate_3d_mat4x4(mat4x4 *matrix, float x, float y, float z, float x_theta, float y_theta, float z_theta)
{
    euler_rotation_mat4x4(matrix, x_theta, y_theta, z_theta);
    matrix->vals[0 + 3*4] = x;
    matrix->vals[1 + 3*4] = y;
    matrix->vals[2 + 3*4] = z;
}
