#version 420

#block Standard3D

layout(location = 0) in vec4 vPosition;

void main(void)
{
    gl_Position = vPosition * transpose(mvp_matrix);
    // gl_Position = vPosition;
}
