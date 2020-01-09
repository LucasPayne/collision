#version 420

in vec4 vPosition;
out vec4 fColor;

void main()
{
    gl_Position = vPosition;
    fColor = vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);
}
