/*================================================================================
   Matrix mathematics module
================================================================================*/
#ifndef HEADER_DEFINED_MATRIX_MATHEMATICS
#define HEADER_DEFINED_MATRIX_MATHEMATICS

typedef struct Matrix3x3f_s {
    float vals[3 * 3];
} Matrix3x3f;

typedef struct Matrix4x4f_s {
    float vals[4 * 4];
} Matrix4x4f;

void euler_rotation_matrix4x4f(Matrix4x4f *matrix, float theta_x, float theta_y, float theta_z);
void euler_rotate_matrix4x4f(Matrix4x4f *matrix, float theta_x, float theta_y, float theta_z);
void x_angle_rotation_matrix4x4f(Matrix4x4f *matrix, float theta);
void y_angle_rotation_matrix4x4f(Matrix4x4f *matrix, float theta);
void z_angle_rotation_matrix4x4f(Matrix4x4f *matrix, float theta);
void identity_matrix4x4f(Matrix4x4f *matrix);
void print_matrix4x4f(Matrix4x4f *matrix);
void right_multiply_matrix4x4f(Matrix4x4f *matrix, Matrix4x4f *B);
void multiply_matrix4x4f(Matrix4x4f *to, Matrix4x4f *A, Matrix4x4f *B);
void translate_matrix4x4f(Matrix4x4f *matrix, float x, float y, float z);
void copy_matrix4x4f(Matrix4x4f *new, Matrix4x4f *copy_from);

void identity_matrix3x3f(Matrix3x3f *matrix);
void right_multiply_matrix3x3f(Matrix3x3f *matrix, Matrix3x3f *B);
void translate_rotate_2d_matrix3x3f(Matrix3x3f *matrix, float x, float y, float theta);

void euler_rotation_matrix3x3f(Matrix3x3f *matrix, float theta_x, float theta_y, float theta_z);
void x_angle_rotate_matrix3x3f(Matrix3x3f *matrix, float theta);
void y_angle_rotate_matrix3x3f(Matrix3x3f *matrix, float theta);
void z_angle_rotate_matrix3x3f(Matrix3x3f *matrix, float theta);

void translate_rotate_3d_matrix4x4f(Matrix4x4f *matrix, float x, float y, float z, float x_theta, float y_theta, float z_theta);

#endif // HEADER_DEFINED_MATRIX_MATHEMATICS


