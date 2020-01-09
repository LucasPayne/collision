#version 420

// pre-preprocessor #include this
layout (shared, binding = 1) uniform StandardView {
    float u_aspect_ratio;
    float u_time;
};
uniform StandardViewModel {
    mat4x4 u_mvp_matrix;
};

/* layout (std140, shared, column_major) uniform StandardView { */
/*     mat4x4 u_mvp_matrix; */
/*     float u_aspect_ratio; */
/*     float u_time; */
/* }; */

uniform mat4x4 mvp_matrix;
uniform float aspect_ratio;
uniform float base_diffuse;


// Non-standard uniforms
uniform float multiplier;
uniform bool grayscale;


layout (location = 0) in vec4 vPosition;
/* layout (location = 1) in vec4 vColor; */
/* layout (location = 101) in bool vPointSprite; */

out vProcessed {
    vec4 fColor;
};

void main(void)
{
    

    gl_Position = vPosition * mvp_matrix;
    gl_Position.w = -gl_Position.z;
    gl_Position.x *= aspect_ratio;

    /* fColor = vec4(0.5, 0.5, 0.5, 1.0); */

    float d = exp(0.1 * vPosition.z - 1);
    fColor = vec4(0.2, 0.2, d, 1.0);
    fColor.rgb *= base_diffuse * u_time;

    /* if (grayscale) { */
    /*     float avg = (fColor.r + fColor.g + fColor.b)/3.0; */
    /*     /1* fColor.rgb = vec3(avg, avg, avg, fColor.a); *1/ */
    /* } */
}
