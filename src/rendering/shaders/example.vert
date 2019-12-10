#version 420

uniform mat4x4 mvp_matrix;
uniform float aspect_ratio;

layout (location = 0) in vec4 vPosition;

out vec4 fColor;

void main(void)
{
    gl_Position = vPosition * mvp_matrix;
    gl_Position.w = -gl_Position.z;
    gl_Position.x *= aspect_ratio;

    /* fColor = vec4(0.5, 0.5, 0.5, 1.0); */
    
    float d = exp(0.1 * vPosition.z - 1);
    fColor = vec4(0.2, 0.2, d, 1.0);
}
