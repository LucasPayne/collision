/*--------------------------------------------------------------------------------
    Vertex shader for shadow passes.
--------------------------------------------------------------------------------*/
#version 420

// Using active_shadow_matrix in Lights block.
#block Lights

layout (location = 0) in vec4 vPosition;

void main(void)
{
    gl_Position = active_shadow_matrix * vPosition;
}
