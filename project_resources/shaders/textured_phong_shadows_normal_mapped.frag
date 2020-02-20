/*--------------------------------------------------------------------------------
todo:
Specular, material properties.
Complete the classical Phong lighting model.
--------------------------------------------------------------------------------*/

#version 420
#block Standard3D
#block Lights

layout (std140) uniform MaterialProperties {
    int dummy;
};

uniform sampler2D diffuse_map;
uniform sampler2D normal_map;

in vOut {
    vec4 fPosition;
    vec2 fTexCoord;
    vec3 fNormal;
    vec4 fDirectionalLightShadowCoord[MAX_NUM_DIRECTIONAL_LIGHTS * NUM_FRUSTUM_SEGMENTS];
    vec3 fTangentSpaceDirectionalLightDirections[MAX_NUM_DIRECTIONAL_LIGHTS];
};

out vec4 color;

void main(void)
{
    //--------------------------------------------------------------------------------
    // Phong lighting.
    //--------------------------------------------------------------------------------
    // Start the color off at the ambient intensity.
    float ambient = 0.2;
    color = vec4(vec3(ambient), 1);

    //--------------------------------------------------------------------------------
    // Shadows.
    //--------------------------------------------------------------------------------
    // Calculate which frustum-segment the fragment is in, for cascaded shadow mapping.
    int segment_index = 0;
    for (int i = 0; i < NUM_FRUSTUM_SEGMENTS; i++) {
        if (dot(fPosition.xyz - camera_position, camera_direction) > shadow_map_segment_depths[i]) {
            segment_index = i;
        }
    }
    
    //--------------------------------------------------------------------------------
    // Directional lights.
    //--------------------------------------------------------------------------------
    for (int i = 0; i < num_directional_lights; i++) {
        //--------------------------------------------------------------------------------
        // Phong lighting.
        //--------------------------------------------------------------------------------
        float cos_theta;
        vec3 normal;

        // Calculate cosine of angle between normal and light.
        normal = normalize(texture(normal_map, fTexCoord).rgb * 2 - 1);
        cos_theta = dot(normal, -fTangentSpaceDirectionalLightDirections[i]);
        //--------------------------------------------------------------------------------
        // Shadows.
        //--------------------------------------------------------------------------------
        float light_factor;

        // Get the interpolated shadow coordinate for the relevant frustum-segment.
        // This coordinate is used for look-up in this segment's shadow map quadrant, to test
        // if the directional light reaches this fragment.
        vec4 shadow_coord = fDirectionalLightShadowCoord[4*i + segment_index];

        // Perform perspective division. ---- w is probably always 1.
        vec3 uvd_coord = shadow_coord.xyz / shadow_coord.w;

        // Shadow acne biasing. This is just a hack heuristic.
        uvd_coord.z += -max((1 - abs(cos_theta)) * 0.005, 0.0005);

        // Look up the relevant shadow map value.
        light_factor = texture(directional_light_shadow_maps[i], uvd_coord.xyz);
        
        // Account for the large amount of cases where the UV is outside of the quadrant.
        // These locations should be lit.
        if (uvd_coord.x < 0.5 * (segment_index % 2) || uvd_coord.x > 0.5 * (segment_index % 2) + 0.5)
            light_factor = 1;
        if (uvd_coord.y < 0.5 * (segment_index / 2) || uvd_coord.y > 0.5 * (segment_index / 2) + 0.5)
            light_factor = 1;
        //--------------------------------------------------------------------------------
        //--------------------------------------------------------------------------------
        // Phong lighting.
        //--------------------------------------------------------------------------------
        float intensity = (1 - ambient) * max(0, cos_theta);
        color += light_factor * vec4(vec3(intensity), 0);
    }

    //--------------------------------------------------------------------------------
    // Texturing.
    //--------------------------------------------------------------------------------
    // Modulate by the texture color or the flat color.
    color *= texture(diffuse_map, fTexCoord);
    //--------------------------------------------------------------------------------
    // Fragment discard for cutout textures.
    //--------------------------------------------------------------------------------
    if (color.a < 0.1) discard;
}
