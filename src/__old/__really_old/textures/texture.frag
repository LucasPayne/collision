#version 420

uniform sampler2D tex;

in vec2 ftexture_coord;

out vec4 color;

void main(void)
{
    color = texture(tex, ftexture_coord);
}
