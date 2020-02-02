/*--------------------------------------------------------------------------------
    Textured phong + shadows, and plastering.
fragment shader
--------------------------------------------------------------------------------*/
#version 420

#block Standard3D
#block Lights

#block Plastering

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

    vec4 fPlasterCoord[MAX_NUM_PLASTERS];
};
out vec4 color;

void main(void)
{
    float ambient = 0.05;
    color = vec4(vec3(ambient), 1);

    for (int i = 0; i < num_directional_lights; i++) {

        float cos_theta = dot(fNormal, -directional_lights[i].direction); // cosine of angle between normal and light

        vec3 uvd_coord = fDirectionalLightShadowCoord[i].xyz / fDirectionalLightShadowCoord[i].w;

        // Shadow acne biasing. This is just a hack heuristic.
        uvd_coord.z += -max((1 - abs(cos_theta)) * 0.005, 0.0005);
        float light_factor = 0.0;

        vec2 texel_size = 1 / textureSize(directional_light_shadow_maps[i], 0);
        const int outward = 0; // change this to change number of surrounding shadow map lookups
        for (int horiz = -outward; horiz <= outward; horiz++) {
            for (int vert = -outward; vert <= outward; vert++) {
                float val = texture(directional_light_shadow_maps[i], vec3(uvd_coord.x + texel_size.x * horiz, uvd_coord.y + texel_size.y * vert, uvd_coord.z));
                light_factor += val;
            }
        }
        light_factor /= (outward + 1) * (outward + 1);
        float intensity = (1 - ambient) * max(0, cos_theta);
        color += light_factor * vec4(vec3(intensity), 0);
    }
    for (int i = 0; i < num_point_lights; i++) {
        float intensity = (1 - ambient) * max(0, dot(-normalize(fPosition.xyz - point_lights[i].position), fNormal));
        color += vec4(vec3(intensity), 0);
    }

    if (use_flat_color) color *= flat_color;
    else color *= texture(diffuse_map, fTexCoord);

    for (int i = 0; i < MAX_NUM_PLASTERS; i++) {
        if (!plasters[i].is_active) continue;
        vec4 got_color = textureProj(plaster_color[i], fPlasterCoord[i]);
        if (got_color != vec4(1,0,1,1)) {
            color = got_color;
        }
    }
}
