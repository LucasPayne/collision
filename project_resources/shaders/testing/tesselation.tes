/*--------------------------------------------------------------------------------
    Testing tesselation.
evaluation shader

http://voxels.blogspot.com/2011/09/tesselation-shader-tutorial-with-source.html
--------------------------------------------------------------------------------*/
#version 420

layout (quads, equal_spacing, ccw) in;

vec4 interpolate(in vec4 v0, in vec4 v1, in vec4 v2, in vec4 v3)
{
    // bilinear interpolation of a quad.
    vec4 a = mix(v0, v1, gl_TessCoord.x);
    vec4 b = mix(v2, v3, gl_TessCoord.x);
    return mix(a, b, gl_TessCoord.y);
}

void main(void)
{
    gl_Position = interpolate(gl_in[0].gl_Position,
                              gl_in[1].gl_Position,
                              gl_in[2].gl_Position,
                              gl_in[3].gl_Position);
}
