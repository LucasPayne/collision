/*--------------------------------------------------------------------------------
This code was generated with the gen_shader_blocks tool, for synchronizing
definitions between glsl and C.
--------------------------------------------------------------------------------*/
#ifndef SHADER_BLOCK_HEADER_DEFINED_LIGHTS
#define SHADER_BLOCK_HEADER_DEFINED_LIGHTS
#define MAX_NUM_DIRECTIONAL_LIGHTS 8
#define MAX_NUM_POINT_LIGHTS 8

char **ShaderBlockSamplerNames_Lights[] = {
}; // ShaderBlockSamplerNames_Lights

struct ShaderBlockStruct_Lights_DirectionalLight { //size: 112
    mat4x4 shadow_matrix;    //offset: 0, alignment: 16, C_type_size: 64
    vec3 half_vector;    //offset: 64, alignment: 16, C_type_size: 12
    char ___std140_pad2[4];
    vec3 direction;    //offset: 80, alignment: 16, C_type_size: 12
    char ___std140_pad3[4];
    vec4 color;    //offset: 96, alignment: 16, C_type_size: 16
};
struct ShaderBlockStruct_Lights_PointLight { //size: 48
    float cubic_attenuation;    //offset: 0, alignment: 4, C_type_size: 4
    float quadratic_attenuation;    //offset: 4, alignment: 4, C_type_size: 4
    float linear_attenuation;    //offset: 8, alignment: 4, C_type_size: 4
    char ___std140_pad3[4];
    vec3 position;    //offset: 16, alignment: 16, C_type_size: 12
    char ___std140_pad4[4];
    vec4 color;    //offset: 32, alignment: 16, C_type_size: 16
};

ShaderBlockID ShaderBlockID_Lights;
typedef struct ShaderBlock_Lights_s {
    mat4x4 active_shadow_matrix;    //offset: 0, alignment: 16, C_type_size: 64
    int num_directional_lights;    //offset: 64, alignment: 4, C_type_size: 4
    int num_point_lights;    //offset: 68, alignment: 4, C_type_size: 4
    char ___std140_pad3[8];
    struct ShaderBlockStruct_Lights_DirectionalLight directional_lights[8];    //offset: 80, alignment: 16, C_type_size: 112
    struct ShaderBlockStruct_Lights_PointLight point_lights[8];    //offset: 192, alignment: 16, C_type_size: 48
} ShaderBlock_Lights;

#endif // SHADER_BLOCK_HEADER_DEFINED_LIGHTS
