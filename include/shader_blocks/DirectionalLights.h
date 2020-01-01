/*--------------------------------------------------------------------------------
This code was generated with the gen_shader_blocks utility, for synchronizing
definitions between glsl and C.
--------------------------------------------------------------------------------*/
#ifndef SHADER_BLOCK_HEADER_DEFINED_DIRECTIONALLIGHTS
#define SHADER_BLOCK_HEADER_DEFINED_DIRECTIONALLIGHTS
#define MAX_NUM_DIRECTIONAL_LIGHTS 32

typedef struct DirectionalLights_DirectionalLight_s {
    vec4 color;
    vec3 direction;
    bool is_active;
} DirectionalLights_DirectionalLight;

ShaderBlockID ShaderBlockID_DirectionalLights;
typedef struct ShaderBlockStruct_DirectionalLights_s {
    DirectionalLight directional_lights[MAX_NUM_DIRECTIONAL_LIGHTS];
} ShaderBlockStruct_DirectionalLights;

#endif // SHADER_BLOCK_HEADER_DEFINED_DIRECTIONALLIGHTS
