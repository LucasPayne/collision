#version 420

out vertex {
    vec2 fTexCoord;
    vec4 fModelPosition;
    vec3 fModelNormal;
};
layout (location = 0) in vec4 vPosition;
layout (location = 2) in vec3 vNormal;
layout (location = 3) in vec2 vTexCoord;

void main(void)
{
    fModelPosition = vPosition;
    fModelNormal = vNormal;
    fTexCoord = vTexCoord;
}
