/*--------------------------------------------------------------------------------
    Dummy fragment shader for shadow passes.
--------------------------------------------------------------------------------*/
#version 420

layout (location = 0) out vec4 color; // why set location?
void main(void)
{
    gl_FragDepth = gl_FragCoord.z; // this is already done implicity.
}
