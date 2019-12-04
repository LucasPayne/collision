#version 420

in vec4 fColor;
in vec4 fPosition;

out vec4 color;

void main(void)
{
#define OPT 1
#if OPT == 1
    color.r = gl_FragCoord.x / 500.0;
    color.g = gl_FragCoord.y / 500.0;
    color.b = gl_FragCoord.z / 500.0;
    color.a = 1.0;
#elif OPT == 2
    color.r = (fPosition.x + 1)/2.0;
    color.g = (fPosition.y + 1)/2.0;
    color.b = (fPosition.z + 1)/2.0;
    color.a = 1.0;
#elif OPT == 3
    // What is gl_FragColor?
    gl_FragColor = vec4(0.5, 0.5, 0.5, 1.0);
#endif
}
