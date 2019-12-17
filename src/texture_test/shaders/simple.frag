#version 420

layout (std140) uniform StandardLoopWindow {
    float aspect_ratio;
    float time;
};

in vOut {
    vec4 fColor;
    vec2 fTexCoord;
};
out vec4 color;

uniform sampler2D diffuse_map;

void main(void)
{
    color = texture(diffuse_map, fTexCoord);
    /* color = fColor * texture(diffuse_map, fTexCoord); */
    /* color = fColor; */
}
