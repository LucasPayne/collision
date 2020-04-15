#version 420

layout(location = 0) in vec4 vPosition;
out vertex {
    vec3 fPosition;
};

void main(void)
{
    fPosition = vPosition.xyz;
}
