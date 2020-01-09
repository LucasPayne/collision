#version 420

layout (std140) uniform MaterialProperties {
    vec4 line_color;
    float on_length;
    float off_length;
};

out vec4 color;

in vOut {
    float lerp;
};

void main(void)
{
    color = vec4(lerp, 0, 0, 1);
    /* color = line_color * (int(lerp * 200) % 2); */
}
