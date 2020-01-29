/*--------------------------------------------------------------------------------
    This variant computes the normals as outward from the model position.
    This is mainly for testing with models that do not have normals.
--------------------------------------------------------------------------------*/
#version 420
#block Standard3D

out vOut {
    vec2 fTexCoord;
    vec4 fPosition;
    vec3 fNormal;
};

// #vertex_format 3U
layout (location = 0) in vec4 vPosition;
layout (location = 3) in vec2 vTexCoord;

void main(void)
{
    gl_Position = mvp_matrix * vPosition; // Interpolate the position of the fragment in the canonical view volume.
    fPosition = model_matrix * vPosition; // Interpolate the position of the fragment in global coordinates.
    fTexCoord = vTexCoord;

    fNormal = (normal_matrix * vec4(vPosition - model_position, 1)).xyz;
}
