#version 420

layout (std140) uniform StandardLoopWindow {
    float aspect_ratio;
    float time;
};

in vOut {
    vec4 fColor;
    vec2 fTexCoord;
    vec3 fPosition;
};
out vec4 color;

uniform sampler2D diffuse_map;
uniform sampler2D diffuse_map2;

void main(void)
{
    /* color = (texture(diffuse_map, fTexCoord) + texture(diffuse_map2, fTexCoord)) / 2 + fColor; */
    color = (texture(diffuse_map, fTexCoord) + texture(diffuse_map2, fTexCoord)) / 2;

    if (fract(fPosition.x * fPosition.y * 0.03) + fract(fPosition.y * 0.03) < 0.5) {
        discard;
    }

    /* color.r *= (sin(10*time)*2 - 1); */

    /* color = fColor * texture(diffuse_map, fTexCoord); */
    /* color = fColor; */
}
