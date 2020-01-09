#version 420

#define RENDERTARGET_DEFAULT_COLOR 0

uniform sampler2D diffuse_map;

in VertexOut {
    vec2 fTexCoord;
    vec2 fScreenPos;
};

layout (location = 0) out vec4 color;

uniform float time;

void main(void)
{
    /* color = vec4(base,0,0, 1.0); */

    /* float r = texture(diffuse_map, fTexCoord).r; */
    /* color += vec4(r,r,r, 1); */

    /* color += vec4(fTexCoord.x, fTexCoord.y, 0.0, 0.0); */
    /* color = vec4(fTexCoord, 0, 1); */

    color = texture(diffuse_map, fTexCoord);
}
