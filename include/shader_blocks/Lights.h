/*--------------------------------------------------------------------------------
This code was generated with the gen_shader_blocks tool, for synchronizing
definitions between glsl and C.
--------------------------------------------------------------------------------*/
#ifndef SHADER_BLOCK_HEADER_DEFINED_LIGHTS
#define SHADER_BLOCK_HEADER_DEFINED_LIGHTS
#define MAX_NUM_DIRECTIONAL_LIGHTS 32
#define MAX_NUM_SPOTLIGHTS 32

struct ShaderBlockStruct_Lights_DirectionalLight { //size: 48
    vec4 color;    //offset: 0, alignment: 16, C_type_size: 16
    bool is_active;    //offset: 16, alignment: 1, C_type_size: 1
    char ___std140_pad2[15];
    vec3 direction;    //offset: 32, alignment: 16, C_type_size: 12
};

ShaderBlockID ShaderBlockID_Lights;
typedef struct ShaderBlock_Lights_s {
    struct ShaderBlockStruct_Lights_DirectionalLight directional_lights[32];    //offset: 0, alignment: 16, C_type_size: 48
} ShaderBlock_Lights;

#endif // SHADER_BLOCK_HEADER_DEFINED_LIGHTS
