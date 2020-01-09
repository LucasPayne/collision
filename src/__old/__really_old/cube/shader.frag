#version 420

uniform float time;
uniform sampler2D tex;

in vec2 ftexture_coord;
in vec4 fColor;
out vec4 color;

void main()
{
#define MODE 3
#if MODE == 0
    float average_rgb = (new_color.r + new_color.g + new_color.b) / 3.0;
    color = vec4(average_rgb,
                 average_rgb,
                 average_rgb,
                 1.0);
#elif MODE == 1
    vec4 dec_part = fColor - ivec4(fColor);
    /* float resolution = 5 + (time - int(time)); */
    float resolution = 5;
    vec4 new_color = ivec4(fColor) + ivec4(resolution * dec_part) / resolution;
    color = new_color;
#elif MODE == 2
    color = fColor;
#elif MODE == 3

    /* vec2 locked_coord = ivec2(4 * ftexture_coord) / 4.0; */
    /* color = texture(tex, locked_coord); */
    color = texture(tex, ftexture_coord);
    /* color = vec4(ftexture_coord.x, ftexture_coord.y, 0, 1.0); */
#endif
}
