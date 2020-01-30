#version 420

#block Standard3D
#block Lights

layout (std140) uniform MaterialProperties {
    bool use_flat_color; // Override the texture and just use a flat color.
    vec4 flat_color;
};
uniform sampler2D diffuse_map;

in vOut {
    vec2 fTexCoord;
    vec4 fPosition;
    vec3 fNormal;

    vec4 fDirectionalLightShadowCoord[MAX_NUM_DIRECTIONAL_LIGHTS];
};
out vec4 color;

bool in_light(float depth_map_depth, float fragment_depth)
{
    float epsilon = 0.00001;
    return abs(depth_map_depth - fragment_depth) < epsilon;
}

void main(void)
{
    float ambient = 0.05;
    color = vec4(vec3(ambient), 1);

    for (int i = 0; i < num_directional_lights; i++) {
        float depth = fDirectionalLightShadowCoord[i].z / fDirectionalLightShadowCoord[i].w;
        float texture_depth = textureProj(directional_light_shadow_maps[i], fDirectionalLightShadowCoord[i]).r;
        if (!in_light(texture_depth, depth)) continue;
        float intensity = (1 - ambient) * max(0, dot(fNormal, -directional_lights[i].direction));
        color += vec4(vec3(intensity), 0);
    }
    for (int i = 0; i < num_point_lights; i++) {
        float intensity = (1 - ambient) * max(0, dot(-normalize(fPosition.xyz - point_lights[i].position), fNormal));
        color += vec4(vec3(intensity), 0);
    }

    if (use_flat_color) color *= flat_color;
    else color *= texture(diffuse_map, fTexCoord);
}
