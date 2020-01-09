#version 420

uniform float time;

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec2 texture_coord;

out vec2 ftexture_coord;

void main(void)
{
    gl_Position = vPosition;
#if 0
    ftexture_coord = texture_coord;
#else
    ftexture_coord = vec2(sin(time) * (texture_coord.x - 0.5) + cos(time) * (texture_coord.y - 0.5),
                          cos(time) * (texture_coord.x - 0.5) + -sin(time) * (texture_coord.y - 0.5));
#endif
}
