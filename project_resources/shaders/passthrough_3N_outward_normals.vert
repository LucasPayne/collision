/*--------------------------------------------------------------------------------
    This variant computes the normals as outward from the model position.
    This is mainly for testing with models that do not have normals.
--------------------------------------------------------------------------------*/
#version 420
#block Standard3D

out vOut {
    vec4 fPosition;
    vec3 fNormal;
};

// #vertex_format 3
layout (location = 0) in vec4 vPosition;

void main(void)
{
    gl_Position = mvp_matrix * vPosition; // Interpolate the position of the fragment in the canonical view volume.
    fPosition = model_matrix * vPosition; // Interpolate the position of the fragment in global coordinates.

    fNormal = (normal_matrix * normalize(vec4(vPosition - vec4(model_position, 1)))).xyz;
    //fNormal = vec3(0,1,1);
}
