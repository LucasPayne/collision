#version 420

uniform vec4 flat_color;
uniform float line_width;

out vec4 color;

in vec2 uv;
flat in float u_bounds;
in vec2 fragment_position; // position in screen-space, [-1,1]x[-1,1].
flat in vec2 f_A;
flat in vec2 f_B;

void main(void)
{
    // color = flat_color;
    // color = vec4(0.5*(uv.x + 1), 0.5*(uv.y + 1), 0, 1);

    float d;
    if (uv.x < -u_bounds) {
        d = length(fragment_position - f_A) - 0.5*line_width;
        if (d > 0) discard; //---hack
    } else if (uv.x > u_bounds) {
        d = length(fragment_position - f_B) - 0.5*line_width;
        if (d > 0) discard; //---hack
    } else if (uv.y > 0) {
        d = (uv.y - 1) * 0.5 * line_width;
    } else {
        d = -(uv.y + 1) * 0.5*line_width;
    }
    if (d < 0) {
        color = flat_color;
    } else {
        // discard;
        float decay = 20 * d / line_width;
        color = vec4(flat_color.rgb, flat_color.a * exp(-decay*decay));
        // color = vec4(1,0,1,1);
        // color = vec4(0,0,0,0);
    }
}
