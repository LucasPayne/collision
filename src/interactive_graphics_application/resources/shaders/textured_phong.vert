#version 420

#block Standard3D
#block Lights

out vOut {
    vec2 fTexCoord;
    vec4 fPosition;
    vec3 fNormal;
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

    // fNormal = (model_matrix * vec4(vNormal, 1)).xyz - model_position;
    
    // fNormal = vec3(0,1,0);


    fNormal = (normal_matrix * vec4(vNormal, 1)).xyz;
}
