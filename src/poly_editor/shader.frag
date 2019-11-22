#version 420

uniform float time;

in vec4 fColor;
out vec4 color;

void main()
{
    /* color = vec4(0.3, 0.3, 0.3, 1.0); */
    
    vec4 dec_part = fColor - ivec4(fColor);
    float amount = 10;
    color = ivec4(fColor) + ivec4(dec_part * amount) / amount;
    color *= 1.0 + 0.2 * sin(time * 10);
    // Taking powers?
    /* color *= color; */
    /* color *= color; */
    // cool
    /* color = ivec4(fColor) + ivec4(dec_part * 10 * amount) / amount; */
}
