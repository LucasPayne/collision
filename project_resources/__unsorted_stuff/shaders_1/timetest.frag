#version 420

#block StandardLoopWindow

out vec4 color;

void main(void)
{
    color = vec4(1 + sin(time)/2, 0, 1 + sin(time)/2, 1);
}
