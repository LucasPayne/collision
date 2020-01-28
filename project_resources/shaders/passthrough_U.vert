#version 420

#block Standard3D

layout(location = 0) in vec4 vPosition;
layout(location = 3) in vec2 vTexCoord;

out vOut {
    vec2 fTexCoord;
};

void main(void)
{
// this transpose fixed something, think about this more ...
    gl_Position = vPosition * transpose(mvp_matrix);
    fTexCoord = vTexCoord;
}
