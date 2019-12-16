/*
Phong shading with directional lights and texturing.
*/
#version 420

// #include "StandardGlobal.glh"
layout (std140) uniform StandardGlobal {
    float time;
    float aspect_ratio;
    int screen_width;
    int screen_height;
};

// #include "StandardLights.glh"
struct DirectionalLight {
    bool enabled;
    vec4 color;
    vec3 n_direction; // normalized
    vec3 n_half_vector; // normalized
};
#define MAX_DIRECTIONAL_LIGHTS 3
layout (std140) uniform Lights {
    bool lighting_enabled;
    DirectionLight directional_lights[MAX_DIRECTIONAL_LIGHTS];
};

layout (std140) uniform Properties {
    float specular_power;
    float specular_multiplier;
    vec4 specular_highlight_color;
    bool bump_mapped;
    float specular_surface_color_reflectance;
    float bump_factor;
};
uniform sampler2D diffuse_map;
uniform sampler2D normal_map;

in vOut {
    vec2 fTexCoord;
    vec4 fPosition;
    vec3 fNormal;
};
layout (location = 0) out vec4 color;

void main(void)
{
    vec4 diffuse = vec4(0,0,0,1);
    float specularity = 0;
    for (int i = 0; i < MAX_DIRECTIONAL_LIGHTS; i++) {
        if (!directional_lights[i].enabled) continue;
        DirectionalLight light = directional_lights[i];
        vec3 normal = normalize(fNormal + bump_factor * texture(normal_map, fTexCoord));
        diffuse += light.color * max(0.0, dot(light.n_direction, normal));
        specularity += dot(light.n_half_vector, normal);
    }
    specularitity = specular_multiplier * pow(specularity, specular_power);
    color = texture(diffuse_map, fTexCoord) * diffuse
          + (texture(diffuse_map, fTexCoord) * specular_surface_color_reflectance + specularity) * specular_highlight_color;
}
