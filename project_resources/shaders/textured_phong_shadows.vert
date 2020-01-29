#version 420

#block Standard3D
#block Lights

out vOut {
    vec2 fTexCoord;
    vec4 fPosition;
    vec3 fNormal;

    vec4 fDirectionalLightShadowCoord[MAX_NUM_DIRECTIONAL_LIGHTS];
};

// #vertex_format 3NU
layout (location = 0) in vec4 vPosition;
layout (location = 2) in vec3 vNormal;
layout (location = 3) in vec2 vTexCoord;

void main(void)
{
    gl_Position = mvp_matrix * vPosition; // Interpolate the position of the fragment in the canonical view volume.
    fPosition = model_matrix * vPosition; // Interpolate the position of the fragment in global coordinates.
    fTexCoord = vTexCoord;
    fNormal = (normal_matrix * vec4(vNormal, 1)).xyz;

    for (int i = 0; i < num_directional_lights; i++) {
        mat4x4 model_shadow_matrix = directional_lights[i].shadow_matrix * model_matrix;
        // Normalized device coordinates to UV texture coordinates + depth.
        mat4x4 ndc_to_uvd_matrix = mat4x4(0.5, 0,   0,   0,
                                          0,   0.5, 0,   0,
                                          0,   0,   0.5, 0,
                                          0.5, 0.5, 0.5, 1);
        fDirectionalLightShadowCoord[i] = ndc_to_uvd_matrix * model_shadow_matrix * vPosition;
        // fDirectionalLightShadowCoord[i] = model_shadow_matrix * vPosition;
    }
}
