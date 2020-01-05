/*--------------------------------------------------------------------------------
This code was generated with the gen_shader_blocks utility, for synchronizing
definitions between glsl and C.
--------------------------------------------------------------------------------*/
#ifndef SHADER_BLOCK_HEADER_DEFINED_CONTRIVEDSTUFF
#define SHADER_BLOCK_HEADER_DEFINED_CONTRIVEDSTUFF
#define NUM 1000
#define ANOTHER_NUM 33

struct ShaderBlockStruct_ContrivedStuff_Stuff { //size: 76
    vec3 v4;    //offset: 0, alignment: 16, C_type_size: 12
    char ___std140_pad1[4];
    vec4 v3;    //offset: 16, alignment: 16, C_type_size: 16
    vec3 v2;    //offset: 32, alignment: 16, C_type_size: 12
    char ___std140_pad3[4];
    vec3 v;    //offset: 48, alignment: 16, C_type_size: 12
    char ___std140_pad4[4];
    bool o;    //offset: 64, alignment: 1, C_type_size: 1
    char ___std140_pad5[3];
    int b;    //offset: 68, alignment: 4, C_type_size: 4
    int a;    //offset: 72, alignment: 4, C_type_size: 4
};
struct ShaderBlockStruct_ContrivedStuff_OtherStuff { //size: 156
    ivec2 fish;    //offset: 0, alignment: 8, C_type_size: 8
    char ___std140_pad1[8];
    mat3x3 topology;    //offset: 16, alignment: 16, C_type_size: 48
    mat4x4 mvp_matrix;    //offset: 64, alignment: 16, C_type_size: 64
    vec3 cool;    //offset: 128, alignment: 16, C_type_size: 12
    char ___std140_pad4[4];
    float stuff;    //offset: 144, alignment: 4, C_type_size: 4
    int i;    //offset: 148, alignment: 4, C_type_size: 4
    uint a;    //offset: 152, alignment: 4, C_type_size: 4
};

ShaderBlockID ShaderBlockID_ContrivedStuff;
typedef struct ShaderBlock_ContrivedStuff_s {
    bool ays[1000];    //offset: 0, alignment: 16, C_type_size: 16000
    struct ShaderBlockStruct_ContrivedStuff_Stuff things[1000];    //offset: 16000, alignment: 16, C_type_size: 80000
    vec3 vvvvvv[33];    //offset: 96000, alignment: 16, C_type_size: 528
    bool cool;    //offset: 96528, alignment: 1, C_type_size: 1
    char ___std140_pad4[7];
    vec2 a_vec;    //offset: 96536, alignment: 8, C_type_size: 8
    vec3 a_vec;    //offset: 96544, alignment: 16, C_type_size: 12
} ShaderBlock_ContrivedStuff;

#endif // SHADER_BLOCK_HEADER_DEFINED_CONTRIVEDSTUFF
