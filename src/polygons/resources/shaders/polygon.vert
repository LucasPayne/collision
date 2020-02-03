#version 420

#block Standard3D
layout (location = 0) in vec4 vPosition;

out vOut {
    vec4 uvd;
};

void main(void)
{
    mat4x4 ndc_to_uvd_matrix = mat4x4(0.5, 0,   0,   0,
                                      0,   0.5, 0,   0,
                                      0,   0,   0.5, 0,
                                      0.5, 0.5, 0.5, 1);
    uvd = ndc_to_uvd_matrix * mvp_matrix * vPosition;
    gl_Position = mvp_matrix * vPosition;
}
