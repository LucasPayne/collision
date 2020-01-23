/*--------------------------------------------------------------------------------
This code was generated with the gen_shader_blocks tool, for synchronizing
definitions between glsl and C.
--------------------------------------------------------------------------------*/
#ifndef SHADER_BLOCK_HEADER_DEFINED_LIGHTS
#define SHADER_BLOCK_HEADER_DEFINED_LIGHTS
#define MAX_NUM_DIRECTIONAL_LIGHTS 32
#define MAX_NUM_SPOTLIGHTS 32

struct ShaderBlockStruct_Lights_DirectionalLight { //size: 32
    vec4 color;    //offset: 0, alignment: 16, C_type_size: 16
    vec3 direction;    //offset: 16, alignment: 16, C_type_size: 12
};

ShaderBlockID ShaderBlockID_Lights;
typedef struct ShaderBlock_Lights_s {
    int num_directional_lights;    //offset: 0, alignment: 4, C_type_size: 4
    char ___std140_pad1[12];
    struct ShaderBlockStruct_Lights_DirectionalLight directional_lights[32];    //offset: 16, alignment: 16, C_type_size: 32
} ShaderBlock_Lights;

#endif // SHADER_BLOCK_HEADER_DEFINED_LIGHTS
