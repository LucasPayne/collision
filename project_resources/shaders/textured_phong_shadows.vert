#version 420

#block Standard3D
#block Lights

out vOut {
    vec2 fTexCoord;
    vec4 fPosition;
    vec3 fNormal;

    // The matrices for these shadow coordinates would transform to a quadrant of the UV coordinates of the shadow map instead of the full one.
    // Only one texture will be used per directional light.
    // But if there is one texture, you won't be able to just account for the earliest (highest density) map, since out of bounds UV coordinates will
    // look up the other parts of the texture ...
    vec4 fDirectionalLightShadowCoord[MAX_NUM_DIRECTIONAL_LIGHTS * NUM_FRUSTUM_SEGMENTS];
};

// #vertex_format 3NU
layout (location = 0) in vec4 vPosition;
layout (location = 2) in vec3 vNormal;
layout (location = 3) in vec2 vTexCoord;

void main(void)
{
    gl_Position = mvp_matrix * vPosition;
    fPosition = model_matrix * vPosition;
    fTexCoord = vTexCoord;
    fNormal = (normal_matrix * vec4(vNormal, 1)).xyz;

    for (int i = 0; i < num_directional_lights; i++) {
        for (int j = 0; j < NUM_FRUSTUM_SEGMENTS; j++) {
            mat4x4 model_shadow_matrix = directional_lights[i].shadow_matrices[j] * model_matrix;
            fDirectionalLightShadowCoord[4*i + j] = model_shadow_matrix * vPosition;
        }
    }
}
