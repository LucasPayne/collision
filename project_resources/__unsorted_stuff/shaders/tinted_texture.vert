#version 420

#block Standard3D

out vOut {
    vec2 fTexCoord;
};

// #vertex_format 3U
layout (location = 0) in vec4 vPosition;
layout (location = 3) in vec2 vTexCoord;

void main(void)
{
    gl_Position = mvp_matrix * vPosition;
    fTexCoord = vTexCoord;
}
