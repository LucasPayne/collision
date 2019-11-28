#version 420

in vec4 fColor;

out vec4 color;

void main(void)
{
    /* color = vec4(1.0, 0.0, 0.0, 1.0); */
    color = fColor;
}
