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

typedef Matrix4x4f mat4x4;
typedef Matrix3x3f mat3x3;

// Vector stuff
typedef struct vec4_s {
    float vals[4];
} vec4;
typedef struct vec3_s {
    float vals[3];
} vec3;
vec3 vec3_mul(vec3 a, float x);
vec3 vec3_add(vec3 a, vec3 b);
vec3 vec3_sub(vec3 a, vec3 b);
vec3 vec3_neg(vec3 a);
float vec3_dot(vec3 a, vec3 b);
vec3 vec3_cross(vec3 u, vec3 v);
vec3 vec3_cross(vec3 u, vec3 v);
vec3 vec3_zero(void);
vec3 vec3_normalize(vec3 v);
float vec3_length(vec3 v); 


// Identity matrix
//================================================================================
void identity_matrix4x4f(Matrix4x4f *matrix);

void identity_matrix3x3f(Matrix3x3f *matrix);

// Multiplication
//================================================================================
void right_multiply_matrix4x4f(Matrix4x4f *matrix, Matrix4x4f *B);
void multiply_matrix4x4f(Matrix4x4f *to, Matrix4x4f *A, Matrix4x4f *B);
void right_multiply_by_transpose_matrix4x4f(Matrix4x4f *matrix, Matrix4x4f *B);

void right_multiply_matrix3x3f(Matrix3x3f *matrix, Matrix3x3f *B);

vec4 matrix_vec4(Matrix4x4f *matrix, vec4 v);

vec3 matrix4_vec3_normal(Matrix4x4f *matrix, vec3 v);

//================================================================================
// Transformations and coordinate frames
//================================================================================
// Rotation
//--------------------------------------------------------------------------------
void euler_rotate_matrix4x4f(Matrix4x4f *matrix, float theta_x, float theta_y, float theta_z);
void euler_rotation_matrix4x4f(Matrix4x4f *matrix, float theta_x, float theta_y, float theta_z);
void x_angle_rotation_matrix4x4f(Matrix4x4f *matrix, float theta);
void y_angle_rotation_matrix4x4f(Matrix4x4f *matrix, float theta);
void z_angle_rotation_matrix4x4f(Matrix4x4f *matrix, float theta);

void euler_rotation_matrix3x3f(Matrix3x3f *matrix, float theta_x, float theta_y, float theta_z);
void x_angle_rotate_matrix3x3f(Matrix3x3f *matrix, float theta);
void y_angle_rotate_matrix3x3f(Matrix3x3f *matrix, float theta);
void z_angle_rotate_matrix3x3f(Matrix3x3f *matrix, float theta);

// Translation
//--------------------------------------------------------------------------------
void translate_matrix4x4f(Matrix4x4f *matrix, float x, float y, float z);


// Homogeneous transformations and coordinate frames
//--------------------------------------------------------------------------------
void translate_rotate_3d_matrix4x4f(Matrix4x4f *matrix, float x, float y, float z, float x_theta, float y_theta, float z_theta);

void translate_rotate_2d_matrix3x3f(Matrix3x3f *matrix, float x, float y, float theta);

// Memory
//================================================================================
void copy_matrix4x4f(Matrix4x4f *new, Matrix4x4f *copy_from);

// Printing and serialization
//================================================================================
void fprint_matrix4x4f(FILE *file, Matrix4x4f *matrix);
void fprint_matrix3x3f(FILE *file, Matrix3x3f *matrix);
void print_matrix4x4f(Matrix4x4f *matrix);
void print_matrix3x3f(Matrix3x3f *matrix);

void print_vec4(vec4 v);

vec4 new_vec4(float x, float y, float z, float w);
vec3 new_vec3(float x, float y, float z);

//----changing matrix module to stuff more like this.
vec3 matrix_vec3(mat3x3 m, vec3 v);
mat3x3 transpose_mat3x3(mat3x3 m);
mat3x3 rotation_part_rigid_mat4x4(mat4x4 m);
vec3 translation_vector_rigid_mat4x4(mat4x4 m);
mat4x4 invert_rigid_mat4x4(mat4x4 m);

mat4x4 euler_angles_mat4x4(float theta_x, float theta_y, float theta_z);
mat4x4 identity_mat4x4(void);

vec4 vec3_to_vec4(vec3 v);
vec3 vec4_to_vec3(vec4 v);

float vec3_square_dist(vec3 a, vec3 b);
float vec3_dist(vec3 a, vec3 b);

#endif // HEADER_DEFINED_MATRIX_MATHEMATICS


