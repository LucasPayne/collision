/*--------------------------------------------------------------------------------
    texture: vertex shader
Straightforward texturing in 3D, with nothing extra.
--------------------------------------------------------------------------------*/
#version 420

#block Standard3D

out vOut {
    vec2 fTexCoord;
};

layout (location = 0) in vec4 vPosition;
layout (location = 3) in vec2 vTexCoord;

void main(void)
{
    gl_Position = mvp_matrix * vPosition;
    fTexCoord = vTexCoord;
}
