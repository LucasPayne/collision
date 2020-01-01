#version 420

layout (std140) uniform Standard3D {
    mat4x4 mvp_matrix;
};

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
