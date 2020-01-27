/*--------------------------------------------------------------------------------
    Dummy fragment shader for shadow passes.
--------------------------------------------------------------------------------*/
#version 420

layout (location = 0) out vec4 color; // why set location?
void main(void)
{
    color = vec4(1.0);
}
