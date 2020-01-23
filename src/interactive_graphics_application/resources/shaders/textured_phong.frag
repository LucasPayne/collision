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

    float litness = ambient;
    for (int i = 0; i < MAX_NUM_DIRECTIONAL_LIGHTS; i++) {
        if (directional_lights[i].is_active) {
            // litness += max(0, dot(fNormal, -directional_lights[i].direction));
            litness += (1 - ambient) * max(0, dot(fPosition.xyz - model_position, -directional_lights[i].direction));
        }
    }
    color = texture(diffuse_map, fTexCoord) * litness;

#endif
}
