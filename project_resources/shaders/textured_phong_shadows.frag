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


#if 0
    for (int i = 0; i < num_directional_lights; i++) {

        float cos_theta = dot(fNormal, -directional_lights[i].direction); // cosine of angle between normal and light

        vec3 uvd_coord = fDirectionalLightShadowCoord[i].xyz / fDirectionalLightShadowCoord[i].w;

        // Shadow acne biasing. This is just a hack heuristic.
        uvd_coord.z += -max((1 - abs(cos_theta)) * 0.005, 0.0005);
        float light_factor = 0.0;
#if 0
	float val = texture(directional_light_shadow_maps[i], uvd_coord);
        light_factor = val;
#else
        vec2 texel_size = 1 / textureSize(directional_light_shadow_maps[i], 0);
        const int outward = 0;
        for (int horiz = -outward; horiz <= outward; horiz++) {
            for (int vert = -outward; vert <= outward; vert++) {
                float val = texture(directional_light_shadow_maps[i], vec3(uvd_coord.x + texel_size.x * horiz, uvd_coord.y + texel_size.y * vert, uvd_coord.z));
                light_factor += val;
            }
        }
        light_factor /= (outward + 1) * (outward + 1);
#endif
        float intensity = (1 - ambient) * max(0, cos_theta);
        color += light_factor * vec4(vec3(intensity), 0);
    }
    for (int i = 0; i < num_point_lights; i++) {
        float intensity = (1 - ambient) * max(0, dot(-normalize(fPosition.xyz - point_lights[i].position), fNormal));
        color += vec4(vec3(intensity), 0);
    }

    if (use_flat_color) color *= flat_color;
    else color *= texture(diffuse_map, fTexCoord);
#endif

    vec4 segment_colors[] = {
        vec4(1,0,0,1),
        vec4(0,1,0,1),
        vec4(0,0,1,1),
        vec4(1,0,1,1),
    };
    vec4 add_color = vec4(0,0,0,1);
    for (int i = 0; i < NUM_FRUSTUM_SEGMENTS; i++) {
        if (dot(fPosition.xyz - camera_position, camera_direction) > shadow_map_segment_depths[i]) {
        // if (dot(fPosition.xyz - camera_position, camera_direction) > 50 * i) {
            add_color = segment_colors[i];
        }
    }
    color += add_color;
}
