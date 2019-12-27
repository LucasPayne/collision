#version 420

layout (std140) uniform StandardLoopWindow {
    float aspect_ratio;
    float time;
};
layout (std140) uniform Standard3D {
    mat4x4 mvp_matrix;
};

layout (location = 0) in vec4 vPosition;
layout (location = 3) in vec2 vTexCoord;

out vOut {
    vec4 fColor;
    vec2 fTexCoord;
    vec3 fPosition;
};

void main(void)
{
    gl_Position = mvp_matrix * vPosition;

    fColor = vec4(0,0,0,1);
    /* fColor = vec4(1,cos(time),0.5,1); */
    /* fTexCoord = vec2(vTexCoord.x * cos(time) + vTexCoord.y * sin(time), */
    /*                  -vTexCoord.x * sin(time) + vTexCoord.y * cos(time)); */
    fTexCoord = vTexCoord;
    fPosition = gl_Position.xyz;
}
