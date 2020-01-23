#version 420

#block Lights

layout (std140) uniform MaterialProperties {
    int specular;
};
uniform sampler2D diffuse_map;

in vOut {
    vec2 fTexCoord;
    vec4 fPosition;
    vec3 fNormal;
};
out vec4 color;

void main(void)
{
    color = texture(diffuse_map, fTexCoord);
}
