/*--------------------------------------------------------------------------------
--------------------------------------------------------------------------------*/
#version 420
#block StandardLoopWindow

in vOut {
    vec3 fNormal;
    vec3 fPosition;
} vOuts[];
out tesselationOut {
    vec3 tNormal;
    vec3 tPosition;
} vertex_outputs[];

layout (vertices = 3) out;

void main(void)
{
    // const float tesselation_level = 8.0;
    float tesselation_level = 10 * sin(time) + 11;

    gl_TessLevelOuter[0] = tesselation_level;
    gl_TessLevelOuter[1] = tesselation_level;
    gl_TessLevelOuter[2] = tesselation_level;
    gl_TessLevelInner[0] = tesselation_level;
    gl_TessLevelInner[1] = tesselation_level;

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    vertex_outputs[gl_InvocationID].tNormal = vOuts[gl_InvocationID].fNormal;
    vertex_outputs[gl_InvocationID].tPosition = vOuts[gl_InvocationID].fPosition;
}
