
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
    gl_Position = mvp_matrix * vPosition;
    fTexCoord = vTexCoord;
    fPosition = vPosition * transpose(model_matrix);
    fNormal = (vec4(vNormal.xyz, 1) * inverse(transpose(model_matrix))).xyz;
}
