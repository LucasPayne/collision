#version 420

layout (std140) uniform StandardLoopWindow {
    float aspect_ratio;
    float time;
};

in vOut {
    vec4 fColor;
};
out vec4 color;

void main(void)
{
    color = fColor;
}
