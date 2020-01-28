#version 420

#block Standard3D

layout (location = 0) in vec4 vPosition;
layout (location = 3) in vec2 vTexCoord;

out vOut {
    vec2 fTexCoord;
};

void main(void)
{   
    gl_Position = mvp_matrix * vPosition;
    fTexCoord = vTexCoord;
}
