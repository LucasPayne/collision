/*--------------------------------------------------------------------------------
This code was generated with the gen_shader_blocks tool, for synchronizing
definitions between glsl and C.
--------------------------------------------------------------------------------*/
#ifndef SHADER_BLOCK_HEADER_DEFINED_LIGHTS
#define SHADER_BLOCK_HEADER_DEFINED_LIGHTS
#define MAX_NUM_DIRECTIONAL_LIGHTS 32
#define MAX_NUM_SPOTLIGHTS 32

struct ShaderBlockStruct_Lights_DirectionalLight { //size: 33
    vec4 color;    //offset: 0, alignment: 16, C_type_size: 16
    vec3 direction;    //offset: 16, alignment: 16, C_type_size: 12
    char ___std140_pad2[4];
    bool is_active;    //offset: 32, alignment: 1, C_type_size: 1
};
struct ShaderBlockStruct_Lights_SpotLight { //size: 76
    float cubic_attenuation;    //offset: 0, alignment: 4, C_type_size: 4
    char ___std140_pad1[12];
    vec4 color;    //offset: 16, alignment: 16, C_type_size: 16
    float quadratic_attenuation;    //offset: 32, alignment: 4, C_type_size: 4
    char ___std140_pad3[12];
    vec3 position;    //offset: 48, alignment: 16, C_type_size: 12
    char ___std140_pad4[4];
    float pulse_rate;    //offset: 64, alignment: 4, C_type_size: 4
    bool is_active;    //offset: 68, alignment: 1, C_type_size: 1
    char ___std140_pad6[3];
    float linear_attenuation;    //offset: 72, alignment: 4, C_type_size: 4
};

ShaderBlockID ShaderBlockID_Lights;
typedef struct ShaderBlock_Lights_s {
    bool lighting_enabled;    //offset: 0, alignment: 1, C_type_size: 1
    char ___std140_pad1[15];
    struct ShaderBlockStruct_Lights_DirectionalLight directional_lights[32];    //offset: 16, alignment: 16, C_type_size: 1536
    struct ShaderBlockStruct_Lights_SpotLight spotlights[32];    //offset: 1552, alignment: 16, C_type_size: 2560
} ShaderBlock_Lights;

#endif // SHADER_BLOCK_HEADER_DEFINED_LIGHTS
