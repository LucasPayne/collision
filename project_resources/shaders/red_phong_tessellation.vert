#version 420

layout(location = 0) in vec4 vPosition;
layout(location = 2) in vec3 vNormal;

out vOut {
    vec3 fNormal;
    vec3 fPosition;
};

void main(void)
{
    fPosition = vPosition.xyz;
    fNormal = vNormal;
}
