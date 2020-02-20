#version 420
#block Standard3D
#block StandardLoopWindow
#block Lights

layout (location = 0) in vec4 vPosition;
layout (location = 2) in vec3 vNormal;
layout (location = 3) in vec2 vTexCoord;
layout (location = 4) in vec3 vTangent;

out vOut {
    vec4 fPosition;
    vec2 fTexCoord;
    vec3 fNormal;
    vec4 fDirectionalLightShadowCoord[MAX_NUM_DIRECTIONAL_LIGHTS * NUM_FRUSTUM_SEGMENTS];
    vec3 fTangentSpaceDirectionalLightDirections[MAX_NUM_DIRECTIONAL_LIGHTS];
};

void main(void)
{
    gl_Position = mvp_matrix * vPosition;

    fPosition = model_matrix * vPosition;
    fTexCoord = vTexCoord;
    fNormal = (normal_matrix * vec4(vNormal, 1)).xyz;

    for (int i = 0; i < num_directional_lights; i++) {
        // Use the shadow matrices for each frustum-segment to create shadow coordinates to
        // interpolate.
        for (int j = 0; j < NUM_FRUSTUM_SEGMENTS; j++) {
            mat4x4 model_shadow_matrix = directional_lights[i].shadow_matrices[j] * model_matrix;
            fDirectionalLightShadowCoord[4*i + j] = model_shadow_matrix * vPosition;
        }
        // Interpolate the tangent-space directions of each directional light. This is used for
        // normal mapping, since the normal map is given in tangent-space.
        vec3 vBinormal = cross(vNormal, vTangent);
        vec3 light_dir = directional_lights[i].direction;
        vec3 tangent_space_direction;
        // [ T B N ]^-1, the inverse TBN (tangent-binormal-normal) matrix.
        tangent_space_direction.x = dot(light_dir, vTangent);
        tangent_space_direction.y = dot(light_dir, vBinormal);
        tangent_space_direction.z = dot(light_dir, vNormal);
        fTangentSpaceDirectionalLightDirections[i] = tangent_space_direction;
    }
}
