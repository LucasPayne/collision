/*--------------------------------------------------------------------------------
This code was generated with the gen_shader_blocks tool, for synchronizing
definitions between glsl and C.
--------------------------------------------------------------------------------*/
#ifndef SHADER_BLOCK_HEADER_DEFINED_STANDARD3D
#define SHADER_BLOCK_HEADER_DEFINED_STANDARD3D
static char *ShaderBlockSamplerNames_Standard3D[] = {
}; // ShaderBlockSamplerNames_Standard3D

#define ShaderBlockNumSamplers_Standard3D 0
struct ___ShaderBlockSamplers_Standard3D {
} ShaderBlockSamplers_Standard3D;

ShaderBlockID ShaderBlockID_Standard3D;
typedef struct ShaderBlock_Standard3D_s {
    bool test_toggle;    //offset: 0, alignment: 1, C_type_size: 1
    char ___std140_pad1[3];
    float near_plane;    //offset: 4, alignment: 4, C_type_size: 4
    float far_plane;    //offset: 8, alignment: 4, C_type_size: 4
    char ___std140_pad3[4];
    mat4x4 mvp_matrix;    //offset: 16, alignment: 16, C_type_size: 64
    mat4x4 model_matrix;    //offset: 80, alignment: 16, C_type_size: 64
    mat4x4 vp_matrix;    //offset: 144, alignment: 16, C_type_size: 64
    vec3 model_position;    //offset: 208, alignment: 16, C_type_size: 12
    char ___std140_pad7[4];
    vec3 camera_direction;    //offset: 224, alignment: 16, C_type_size: 12
    char ___std140_pad8[4];
    vec3 camera_position;    //offset: 240, alignment: 16, C_type_size: 12
    char ___std140_pad9[4];
    mat4x4 normal_matrix;    //offset: 256, alignment: 16, C_type_size: 64
} ShaderBlock_Standard3D;

#endif // SHADER_BLOCK_HEADER_DEFINED_STANDARD3D
