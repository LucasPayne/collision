/*
Phong shading with directional lights and texturing.
*/
#version 420

// #include "StandardGlobal.glh"
// Is it at all a bad thing to just have things available to shaders? Since this is a shared block anyway, I think it would
// just be convenient.
layout (std140) uniform StandardGlobal {
    float time;
    float aspect_ratio;
    int screen_width;
    int screen_height;
};
// #include "Standard3D.glh"
layout (std140) uniform Standard3D {
    // View granularity
    mat4x4 view_matrix;
    mat4x4 projection_matrix;
    mat4x4 vp_matrix;
    // Model granularity
    mat4x4 model_matrix;
    vec3 model_position;
    vec3 model_euler_angles;
    mat3x3 normal_matrix; // transform model-relative normals to eye space. No translations.
    mat4x4 mv_matrix; // to view space, no projections, e.g. for lighting calculations
    mat4x4 mvp_matrix;
};
// #include "StandardLights.glh"
struct DirectionalLight {
    bool enabled;
    vec4 color;
    vec3 direction;
    vec3 half_vector;
};
#define MAX_DIRECTIONAL_LIGHTS 3
layout (std140) uniform Lights {
    bool lighting_enabled;
    DirectionLight directional_lights[MAX_DIRECTIONAL_LIGHTS];
};
layout (std140) uniform Properties {
    float specular_power;
    float specular_multiplier;
};
uniform sampler2D diffuse_map;
uniform sampler2D bump_map;

// #include "VertexStreams_3NU"
layout (location = 0) in vec4 vPosition;
layout (location = 2) in vec3 vNormal;
layout (location = 3) in vec2 vTexCoord;

out vOut {
    vec2 fTexCoord;
    vec4 fPosition;
    vec3 fNormal;
};

void main(void)
{
    gl_Position = mvp_matrix * vPosition;
    fPosition = mv_matrix * vPosition;
    fTexCoord = vTexCoord;
    fNormal = normal_matrix * vNormal;
}
