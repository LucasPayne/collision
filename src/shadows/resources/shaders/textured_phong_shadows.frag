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
    // if (dot(camera_position - fPosition.xyz, camera_position - fPosition.xyz) > 20*20) discard;

    float ambient = 0.2;
    color = vec4(ambient, ambient, ambient, 1);
    float specular = 0;

    // Directional lights
    for (int i = 0; i < num_directional_lights; i++) {
        // Diffuse
        float intensity = (1 - ambient) * max(0, dot(fNormal, -directional_lights[i].direction)); // The light's direction should be normal.
        color += directional_lights[i].color * vec4(intensity,intensity,intensity,1);
        // Specular
        specular += max(0, dot(fNormal, directional_lights[i].half_vector));
    }

    // Texture
    color *= texture(diffuse_map, fTexCoord);
    // Specular
    color += vec4(specular, specular, specular, 0);
}
