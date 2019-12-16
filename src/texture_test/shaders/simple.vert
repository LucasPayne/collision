#version 420

layout (std140) uniform StandardLoopWindow {
    float aspect_ratio;
    float time;
};

layout (location = 0) in vec4 vPosition;

out vOut {
    vec4 fColor;
};

void main(void)
{
    gl_Position = vPosition;
    fColor = vec4(1,cos(time),0.5,1);
}
