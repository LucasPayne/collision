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

    vec4 fDirectionalLightShadowCoord[MAX_NUM_DIRECTIONAL_LIGHTS * NUM_FRUSTUM_SEGMENTS];
};
out vec4 color;

void main(void)
{
    float ambient = 0.05;
    color = vec4(vec3(ambient), 1);
    vec4 add_color = vec4(0,0,0,1);

    vec4 segment_colors[] = {
        vec4(1,0,0,1),
        vec4(0,1,0,1),
        vec4(0,0,1,1),
        vec4(1,0,1,1),
    };
    int segment_index = 0;
    for (int i = 0; i < NUM_FRUSTUM_SEGMENTS; i++) {
        if (dot(fPosition.xyz - camera_position, camera_direction) > shadow_map_segment_depths[i]) {
            add_color = 0.5 * segment_colors[i];
            segment_index = i;
        }
    }
    
    for (int i = 0; i < num_directional_lights; i++) {
        vec4 shadow_coord = fDirectionalLightShadowCoord[4*i + segment_index];
        // shadow_coord.y = 1 - shadow_coord.y;

#if 1
        float cos_theta = dot(fNormal, -directional_lights[i].direction); // cosine of angle between normal and light
        vec3 uvd_coord = shadow_coord.xyz / shadow_coord.w; // perspective. ---probably doesn't matter for directional lights.


        // Shadow acne biasing. This is just a hack heuristic.
        uvd_coord.z += -max((1 - abs(cos_theta)) * 0.005, 0.0005);

        float light_factor = texture(directional_light_shadow_maps[i], uvd_coord.xyz);

        float intensity = (1 - ambient) * max(0, cos_theta);
        color += light_factor * vec4(vec3(intensity), 0);
#endif
    }

    // Point lights.
    for (int i = 0; i < num_point_lights; i++) {
        float intensity = (1 - ambient) * max(0, dot(-normalize(fPosition.xyz - point_lights[i].position), fNormal));
        color += vec4(vec3(intensity), 0);
    }

    // Modulate by the texture color or the flat color.
    if (use_flat_color) color *= flat_color;
    else color *= texture(diffuse_map, fTexCoord);
    // Add the tinting color (this can be used for visualization and testing).
    color += add_color;
}
