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
    for (int i = 0; i < MAX_NUM_DIRECTIONAL_LIGHTS; i++) {
        if (directional_lights[i].is_active) {

        }
    }
    color = texture(diffuse_map, fTexCoord);
}
