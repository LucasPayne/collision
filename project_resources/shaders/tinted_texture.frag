#version 420

#block StandardLoopWindow

layout (std140) uniform MaterialProperties {
    vec4 flat_color;
};
uniform sampler2D diffuse_map;

in vOut {
    vec2 fTexCoord;
};
out vec4 color;

void main(void)
{
    color = flat_color + texture(diffuse_map, fTexCoord);
}
