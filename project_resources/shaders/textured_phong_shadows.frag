#version 420

#block Standard3D
#block Lights

layout (std140) uniform MaterialProperties {
    int temp;
};
uniform sampler2D diffuse_map;

in vOut {
    vec2 fTexCoord;
    vec4 fPosition;
    vec3 fNormal;

    vec4 fDirectionalLightShadowCoord[MAX_NUM_DIRECTIONAL_LIGHTS];
};
out vec4 color;

void main(void)
{
    float ambient = 0.2;
    color = vec4(ambient, ambient, ambient, 1);

    // Directional lights
    for (int i = 0; i < num_directional_lights; i++) {
        float intensity = (1 - ambient) * max(0, dot(fNormal, -directional_lights[i].direction));
        float shadow_factor;
        // shadow_factor = textureProj(directional_light_shadow_maps[i], fDirectionalLightShadowCoord[i]);
        shadow_factor = 1;
        color += directional_lights[i].color * vec4(intensity,intensity,intensity,1) * shadow_factor;
    }

    // Texture
#define mode 1
#if mode == 0
    color *= texture(diffuse_map, fTexCoord);
#elif mode == 1
    if (test_toggle) color = texture(test_texture, fTexCoord);
    else color = texture(test_texture_2, fTexCoord);
#elif mode == 2
    color = texture(directional_light_shadow_maps[0], fTexCoord);
#endif
}
