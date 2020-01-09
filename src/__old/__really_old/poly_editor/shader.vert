#version 420

uniform float time;

in vec4 vPosition;
in vec4 vColor;
out vec4 fColor;

void main()
{
    gl_Position = vPosition;
    /* gl_Position.x += sin(time); */
    fColor = vColor;
    /* fColor = vec4(vPosition.x, vPosition.y, vPosition.z, 1.0); */
}
