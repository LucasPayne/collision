/*================================================================================
Paint circles for circular point rendering.
================================================================================*/
#version 420
uniform vec4 flat_color;

out vec4 color;

void main(void)
{
    float x,y;
    x = gl_PointCoord.x - 0.5;
    y = gl_PointCoord.y - 0.5;
    if (x*x + y*y > 0.25) discard;
    color = flat_color;
}

