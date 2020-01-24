/*--------------------------------------------------------------------------------
This code was generated with the gen_shader_blocks tool, for synchronizing
definitions between glsl and C.
--------------------------------------------------------------------------------*/
#ifndef SHADER_BLOCK_HEADER_DEFINED_STANDARD3D
#define SHADER_BLOCK_HEADER_DEFINED_STANDARD3D
ShaderBlockID ShaderBlockID_Standard3D;
typedef struct ShaderBlock_Standard3D_s {
    mat4x4 mvp_matrix;    //offset: 0, alignment: 16, C_type_size: 64
    mat4x4 model_matrix;    //offset: 64, alignment: 16, C_type_size: 64
    vec3 model_position;    //offset: 128, alignment: 16, C_type_size: 12
    char ___std140_pad3[4];
    mat4x4 normal_matrix;    //offset: 144, alignment: 16, C_type_size: 64
} ShaderBlock_Standard3D;

#endif // SHADER_BLOCK_HEADER_DEFINED_STANDARD3D
