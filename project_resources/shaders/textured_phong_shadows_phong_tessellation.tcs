/*--------------------------------------------------------------------------------
Phong-tessellation extension to the textured_phong_shadows material.

tessellation control shader
--------------------------------------------------------------------------------*/
#version 420
#block Lights
layout (vertices = 3) out;

in vertex {
    vec2 fTexCoord;
    vec4 fModelPosition;
    vec3 fModelNormal;
} vertices[];
out controlled_vertex {
    vec2 tTexCoord;
    vec4 tModelPosition;
    vec3 tModelNormal;
} vertex_outputs[];

void main(void)
{
    // Set the tessellation levels.
    const float tesselation_level = 8;
    gl_TessLevelOuter[0] = tesselation_level;
    gl_TessLevelOuter[1] = tesselation_level;
    gl_TessLevelOuter[2] = tesselation_level;
    gl_TessLevelInner[0] = tesselation_level;
    gl_TessLevelInner[1] = tesselation_level;

    // Pass through the vertex unchanged.
    vertex_outputs[gl_InvocationID].tModelNormal = vertices[gl_InvocationID].fModelNormal;
    vertex_outputs[gl_InvocationID].tModelPosition = vertices[gl_InvocationID].fModelPosition;
    vertex_outputs[gl_InvocationID].tTexCoord = vertices[gl_InvocationID].fTexCoord;



    // gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}
