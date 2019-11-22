#version 420

/* uniform float time; */

in vec4 fColor;
in vec4 fPosition;
out vec4 color;

void main(void)
{
    /* color = vec4(fColor.rg, 0.5 + sin(time * 3 / fPosition.z), 1.0); */
    color = fColor;
    /* color = vec4(1.0, 1.0, 0.5, 1.0); */
}
