#version 420

in vProcessed {
    vec4 fColor;
};


// ---- How do fragment shader outputs work?
out vec4 color;

void main(void)
{
    color = fColor;
    /* color = vec4(1.0, 0.0, 0.0, 1.0); */
}
