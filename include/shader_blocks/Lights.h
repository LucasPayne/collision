/*--------------------------------------------------------------------------------
This code was generated with the gen_shader_blocks tool, for synchronizing
definitions between glsl and C.
--------------------------------------------------------------------------------*/
#ifndef SHADER_BLOCK_HEADER_DEFINED_LIGHTS
#define SHADER_BLOCK_HEADER_DEFINED_LIGHTS
#define MAX_NUM_DIRECTIONAL_LIGHTS 32
#define MAX_NUM_POINT_LIGHTS 32

struct ShaderBlockStruct_Lights_DirectionalLight { //size: 32
    vec3 direction;    //offset: 0, alignment: 16, C_type_size: 12
    char ___std140_pad1[4];
    vec4 color;    //offset: 16, alignment: 16, C_type_size: 16
};
struct ShaderBlockStruct_Lights_PointLight { //size: 48
    vec3 position;    //offset: 0, alignment: 16, C_type_size: 12
    char ___std140_pad1[4];
    float cubic_attenuation;    //offset: 16, alignment: 4, C_type_size: 4
    float quadratic_attenuation;    //offset: 20, alignment: 4, C_type_size: 4
    float linear_attenuation;    //offset: 24, alignment: 4, C_type_size: 4
    char ___std140_pad4[4];
    vec4 color;    //offset: 32, alignment: 16, C_type_size: 16
};

ShaderBlockID ShaderBlockID_Lights;
typedef struct ShaderBlock_Lights_s {
    int num_directional_lights;    //offset: 0, alignment: 4, C_type_size: 4
    int num_point_lights;    //offset: 4, alignment: 4, C_type_size: 4
    char ___std140_pad2[8];
    struct ShaderBlockStruct_Lights_DirectionalLight directional_lights[32];    //offset: 16, alignment: 16, C_type_size: 32
    struct ShaderBlockStruct_Lights_PointLight point_lights[32];    //offset: 48, alignment: 16, C_type_size: 48
} ShaderBlock_Lights;

#endif // SHADER_BLOCK_HEADER_DEFINED_LIGHTS
