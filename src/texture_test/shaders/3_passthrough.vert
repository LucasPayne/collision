#version 420

layout (std140) uniform Standard3D {
    mat4x4 mvp_matrix;
};

layout (location = 0) in vec4 vPosition;

void main(void)
{
    gl_Position = mvp_matrix * vPosition;
}
