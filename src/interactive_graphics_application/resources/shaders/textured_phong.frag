#version 420

#block Standard3D
#block Lights

layout (std140) uniform MaterialProperties {
    int specular;
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
#if 0 // testing normals
    if (fNormal.x > 0) discard;
    color = texture(diffuse_map, fTexCoord);
#else
    float ambient = 0.2;
    color = vec4(0.2, 0.2, 0.2, 1);

    // for (int i = 0; i < MAX_NUM_DIRECTIONAL_LIGHTS; i++) {
    for (int i = 0; i < num_directional_lights; i++) {
            // litness += max(0, dot(fNormal, -directional_lights[i].direction));
            //---calculating normals outward from centre of model
            float intensity = (1 - ambient) * max(0, dot(fPosition.xyz - model_position, -directional_lights[i].direction));
            color += directional_lights[i].color * vec4(intensity,intensity,intensity,1);
        // }
    }
    color *= texture(diffuse_map, fTexCoord);

#endif
}
