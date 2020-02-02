/*--------------------------------------------------------------------------------
This code was generated with the gen_shader_blocks tool, for synchronizing
definitions between glsl and C.
--------------------------------------------------------------------------------*/
#ifndef SHADER_BLOCK_HEADER_DEFINED_PLASTERING
#define SHADER_BLOCK_HEADER_DEFINED_PLASTERING
#define MAX_NUM_PLASTERS 8

static char *ShaderBlockSamplerNames_Plastering[] = {
    "plaster[0]",
    "plaster[1]",
    "plaster[2]",
    "plaster[3]",
    "plaster[4]",
    "plaster[5]",
    "plaster[6]",
    "plaster[7]",
}; // ShaderBlockSamplerNames_Plastering

#define ShaderBlockNumSamplers_Plastering 8
struct ___ShaderBlockSamplers_Plastering {
    GLint plaster[8];
} ShaderBlockSamplers_Plastering;

struct ShaderBlockStruct_Plastering_Plaster { //size: 64
    mat4x4 matrix;    //offset: 0, alignment: 16, C_type_size: 64
};

ShaderBlockID ShaderBlockID_Plastering;
typedef struct ShaderBlock_Plastering_s {
    struct ShaderBlockStruct_Plastering_Plaster plasters[8];    //offset: 0, alignment: 16, C_type_size: 64
} ShaderBlock_Plastering;

#endif // SHADER_BLOCK_HEADER_DEFINED_PLASTERING
