#version 420

layout (std140) uniform Standard3D {
    mat4x4 mvp_matrix;
};

layout (location = 0) in vec4 vPosition;
layout (location = 4) in uint vIndex;

out vOut {
    float lerp;
};

void main(void)
{
    /* lerp = float(vIndex % 2); */

    lerp = vIndex/10.0;

    gl_Position = mvp_matrix * vPosition;
}
