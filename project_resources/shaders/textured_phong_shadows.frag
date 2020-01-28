#version 420

#block Standard3D
#block Lights

uniform sampler2DShadow directional_light_shadow_maps[MAX_NUM_DIRECTIONAL_LIGHTS];

layout (std140) uniform MaterialProperties {
    int temp;
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
    float ambient = 0.2;
    color = vec4(ambient, ambient, ambient, 1);
    float specular = 0;

    // Directional lights
    for (int i = 0; i < num_directional_lights; i++) {
        float intensity = (1 - ambient) * max(0, dot(fNormal, -directional_lights[i].direction));
        color += directional_lights[i].color * vec4(intensity,intensity,intensity,1);
    }

    // Texture
    color *= texture(diffuse_map, fTexCoord);
}
