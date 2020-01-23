#version 420

layout (std140) uniform MaterialProperties {
    vec4 flat_color;
};

out vec4 color;

void main(void)
{
    color = flat_color;
}
