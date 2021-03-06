#version 420

#block Standard3D
#block Lights
// For testing. It would be better to have used blocks be autodetected instead of declared in the material type definition.
#block StandardLoopWindow

layout (std140) uniform MaterialProperties {
    vec4 flat_color;
};

in vOut {
    vec4 fPosition;
    vec3 fNormal;
};
out vec4 color;

void main(void)
{
    //if (dot(fPosition.xyz - model_position, fPosition.xyz - model_position) > pow(15*(sin(time)+1), 2)) discard;

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
    // Point lights
    for (int i = 0; i < num_point_lights; i++) {
        vec3 to_light = normalize(point_lights[i].position - fPosition.xyz);
        float intensity = (1 - ambient) * max(0, dot(fNormal, to_light));

        float dist = length(fPosition.xyz - point_lights[i].position);
        float attenuation = exp(-(
            dist * point_lights[i].linear_attenuation +
            dist*dist * point_lights[i].quadratic_attenuation +
            dist*dist*dist * point_lights[i].cubic_attenuation
        ));
        color += vec4(intensity,intensity,intensity,1) * attenuation;
        // Specular
        specular += max(0, dot(fNormal, normalize(to_light + normalize(camera_position - fPosition.xyz))));
    }

    // Flat color
    color *= flat_color;
    // Specular
    color += vec4(specular, specular, specular, 0);
    // testing with a texture
    // color *= texture(test_texture, fPosition.xy * 0.05);
}
