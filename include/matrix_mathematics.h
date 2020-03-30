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
typedef struct vec2_s {
    float vals[2];
} vec2;
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
float vec3_square_length(vec3 v);

vec4 vec4_zero(void);

vec3 vec3_lerp(vec3 a, vec3 b, float t);
// takes r+s+t = 1
vec3 triangle_blend(vec3 a, vec3 b, vec3 c, float r, float s, float t);

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
void print_vec3(vec3 v);

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

vec2 new_vec2(float x, float y);

vec3 mat4x4_vec3(mat4x4 *matrix, vec3 v);

mat3x3 multiply_mat3x3(mat3x3 A, mat3x3 B);

// multiply only the first three components.
vec4 color_mul(vec4 color, float x);
// multiply only the last component.
vec4 color_fade(vec4 color, float x);

#define fill_mat3x3(MATRIX,A,B,C,D,E,F,G,H,I)\
{\
    ( MATRIX ).vals[0] = ( A );\
    ( MATRIX ).vals[1] = ( B );\
    ( MATRIX ).vals[2] = ( C );\
    ( MATRIX ).vals[3] = ( D );\
    ( MATRIX ).vals[4] = ( E );\
    ( MATRIX ).vals[5] = ( F );\
    ( MATRIX ).vals[6] = ( G );\
    ( MATRIX ).vals[7] = ( H );\
    ( MATRIX ).vals[8] = ( I );\
}

#define fill_mat4x4(MATRIX,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O)\
{\
    ( MATRIX ).vals[0] = ( A );\
    ( MATRIX ).vals[1] = ( B );\
    ( MATRIX ).vals[2] = ( C );\
    ( MATRIX ).vals[3] = ( D );\
    ( MATRIX ).vals[4] = ( E );\
    ( MATRIX ).vals[5] = ( F );\
    ( MATRIX ).vals[6] = ( G );\
    ( MATRIX ).vals[7] = ( H );\
    ( MATRIX ).vals[8] = ( I );\
    ( MATRIX ).vals[9] = ( I );\
    ( MATRIX ).vals[10] = ( J );\
    ( MATRIX ).vals[11] = ( K );\
    ( MATRIX ).vals[12] = ( L );\
    ( MATRIX ).vals[13] = ( M );\
    ( MATRIX ).vals[14] = ( N );\
    ( MATRIX ).vals[15] = ( O );\
}

mat3x3 mat3x3_add(mat3x3 A, mat3x3 B);
void mat3x3_orthonormalize(mat3x3 *m);


#endif // HEADER_DEFINED_MATRIX_MATHEMATICS


