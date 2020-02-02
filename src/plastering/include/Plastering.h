/*--------------------------------------------------------------------------------
This code was generated with the gen_shader_blocks tool, for synchronizing
definitions between glsl and C.
--------------------------------------------------------------------------------*/
#ifndef SHADER_BLOCK_HEADER_DEFINED_PLASTERING
#define SHADER_BLOCK_HEADER_DEFINED_PLASTERING
#define MAX_NUM_PLASTERS 2

static char *ShaderBlockSamplerNames_Plastering[] = {
    "plaster_color[0]",
    "plaster_color[1]",
    "plaster_depth[0]",
    "plaster_depth[1]",
}; // ShaderBlockSamplerNames_Plastering

#define ShaderBlockNumSamplers_Plastering 4
struct ___ShaderBlockSamplers_Plastering {
    GLint plaster_color[2];
    GLint plaster_depth[2];
} ShaderBlockSamplers_Plastering;

struct ShaderBlockStruct_Plastering_Plaster { //size: 80
    bool is_active;    //offset: 0, alignment: 1, C_type_size: 1
    char ___std140_pad1[15];
    mat4x4 matrix;    //offset: 16, alignment: 16, C_type_size: 64
};

ShaderBlockID ShaderBlockID_Plastering;
typedef struct ShaderBlock_Plastering_s {
    struct ShaderBlockStruct_Plastering_Plaster plasters[2];    //offset: 0, alignment: 16, C_type_size: 80
} ShaderBlock_Plastering;

#endif // SHADER_BLOCK_HEADER_DEFINED_PLASTERING
