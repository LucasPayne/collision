#version 420

uniform mat4x4 mvp_matrix;
uniform float aspect_ratio;
uniform float frand;
uniform float time;

uniform float model_x;
uniform float model_y;
uniform float model_z;
uniform float light_pos_x;
uniform float light_pos_y;
uniform float light_pos_z;

layout (location = 0) in vec4 vPosition;

out vec4 fColor;

void main(void)
{
    vec4 pos = vPosition;
    gl_Position = mvp_matrix * pos;
    gl_Position.w = -gl_Position.z;
    gl_Position.x *= aspect_ratio;

    
    vec4 light_pos = vec4(light_pos_x, light_pos_y, light_pos_z, 1.0);

    vec4 light_dir_n = normalize(light_pos - gl_Position);
    float light_dist = distance(light_pos, gl_Position);
    vec4 normal_dir_n = normalize(gl_Position - vec4(model_x, model_y, model_z, 1.0));
    float light_dot = dot(light_dir_n, normal_dir_n);

    /* float intensity = exp(-light_dist*0.1) * (0.5 + light_dot/2); */
    float intensity = 0.5 + light_dot/2;

    fColor = vec4(0, intensity, intensity, 1);


    /* float d = exp(0.1 * pos.z); */
    /* fColor = 1 - vec4(d, d, d, 1.0); */
    /* fColor = vec4(exp(0.1*pos.z), exp(0.1*pos.z), exp(0.1*pos.z), 1.0); */
    /* fColor = vec4(exp(0.1*pos.z), exp(0.1*pos.z), exp(0.1*pos.z), 1.0); */
}
