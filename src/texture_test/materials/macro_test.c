
#define uniform_set_mat4x4(SHADER_BLOCK,ENTRY,MATRIX_VALS)\
    ___uniform_set_mat4x4(( ShaderBlockID_ ## SHADER_BLOCK ),\
                          &(( g_shaderblocks[( ShaderBlockID_ ## SHADER_BLOCK )]. ## ENTRY )),\
                          MATRIX_VALS)

uniform_set_mat4x4(SomeStuff, things[j].stuff.matrices[i], &mat.vals);

