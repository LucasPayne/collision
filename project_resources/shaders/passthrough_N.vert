#version 420

#block Standard3D

layout(location = 0) in vec4 vPosition;
layout(location = 2) in vec3 vNormal;

out vOut {
    vec3 fNormal;
};

void main(void)
{
    gl_Position = mvp_matrix * vPosition;
    fNormal = vNormal;
}
