#version 420

#define RENDERTARGET_DEFAULT_COLOR 0

uniform sampler2D diffuse_map;

in VertexOut {
    vec2 fTexCoord;
};

layout (location = 0) out vec4 color;

void main(void)
{
    float r = texture(diffuse_map, fTexCoord).r;
    color = vec4(r,r,r, 1);

    /* color = texture(diffuse_map, fTexCoord); */
}
