/*--------------------------------------------------------------------------------
--------------------------------------------------------------------------------*/
#version 420
#block Lights

layout (vertices = 3) out;

in vOut {
    vec2 fTexCoord;
    vec4 fPosition;
    vec3 fNormal;
    vec4 fDirectionalLightShadowCoord[MAX_NUM_DIRECTIONAL_LIGHTS * NUM_FRUSTUM_SEGMENTS];
} vOuts[];
out tesselationOut {
    vec2 tTexCoord;
    vec4 tPosition;
    vec3 tNormal;
    vec4 tDirectionalLightShadowCoord[MAX_NUM_DIRECTIONAL_LIGHTS * NUM_FRUSTUM_SEGMENTS];
} vertex_outputs[];

void main(void)
{
    const float tesselation_level = 8.0;

    gl_TessLevelOuter[0] = tesselation_level;
    gl_TessLevelOuter[1] = tesselation_level;
    gl_TessLevelOuter[2] = tesselation_level;
    gl_TessLevelInner[0] = tesselation_level;
    gl_TessLevelInner[1] = tesselation_level;

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    vertex_outputs[gl_InvocationID].tTexCoord = vOuts[gl_InvocationID].fTexCoord;
    vertex_outputs[gl_InvocationID].tPosition = vOuts[gl_InvocationID].fPosition;
    vertex_outputs[gl_InvocationID].tNormal   = vOuts[gl_InvocationID].fNormal;
}
