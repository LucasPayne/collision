#version 420

uniform mat4x4 mvp_matrix;
uniform float aspect_ratio;
uniform float frand;
uniform float time;

layout (location = 0) in vec4 vPosition;

out vec4 fColor;

void main(void)
{
    vec4 pos = vPosition;

    gl_Position = mvp_matrix * pos;
    gl_Position.w = -gl_Position.z;

    gl_Position.x *= aspect_ratio;

    float d = exp(0.1 * pos.z);
    fColor = 1 - vec4(d, d, d, 1.0);
    /* fColor = vec4(exp(0.1*pos.z), exp(0.1*pos.z), exp(0.1*pos.z), 1.0); */
    /* fColor = vec4(exp(0.1*pos.z), exp(0.1*pos.z), exp(0.1*pos.z), 1.0); */
}
