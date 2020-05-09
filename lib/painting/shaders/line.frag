#version 420

uniform vec4 flat_color;

out vec4 color;

in vec2 uv;

void main(void)
{
    // color = flat_color;
    color = vec4(0.5*(uv.x + 1), 0.5*(uv.y + 1), 0, 1);
}
