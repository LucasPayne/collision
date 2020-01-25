#version 420

#block Standard3D

layout(location = 0) in vec4 vPosition;

out vOut {
    vec4 fPosition;
};

void main(void)
{
    gl_Position = vPosition * mvp_matrix;
    fPosition = vPosition;
}
