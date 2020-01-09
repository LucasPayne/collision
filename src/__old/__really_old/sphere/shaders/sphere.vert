#version 420

/* uniform mat4x4 model_matrix; */
/* uniform float aspect_ratio; */
/* uniform float time; */

uniform mat4x4 modelview_matrix;

in layout(location = 0) vec4 vPosition;
out vec4 fColor;
out vec4 fPosition;

void main(void)
{
    vec4 pos = vPosition * modelview_matrix;

    // "Perspective"
    gl_Position = vec4(pos.x, pos.y, pos.z, 1.5 + pos.z);
    /* gl_Position.x *= aspect_ratio; */
    fPosition = gl_Position;

    fColor = vec4(0.5 * exp(pos.z), 0.5 * exp(pos.z), 0.5*exp(pos.z), 1.0);
}
