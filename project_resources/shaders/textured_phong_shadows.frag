#version 420

#block Standard3D
#block Lights

layout (std140) uniform MaterialProperties {
    int temp;
};
uniform sampler2D diffuse_map;

in vOut {
    vec2 fTexCoord;
    vec4 fPosition;
    vec3 fNormal;

    vec4 fDirectionalLightShadowCoord[MAX_NUM_DIRECTIONAL_LIGHTS];
};
out vec4 color;

bool in_light(float depth_map_depth, float fragment_depth)
{
    float epsilon = 0.00001;
    return abs(depth_map_depth - fragment_depth) < epsilon;
}

        // float depth = textureProj(directional_light_shadow_maps[i], fDirectionalLightShadowCoord[i]).r;
        // vec4 depth_color = vec4(vec3(depth), 1);
        // if (in_light(depth / gl_FragCoord.w, gl_FragCoord.z / gl_FragCoord.w)) color += vec4(vec3(0.5), 1);

void main(void)
{
    color = vec4(0,0,0,1);
    for (int i = 0; i < num_directional_lights; i++) {
        float depth = fDirectionalLightShadowCoord[i].z / fDirectionalLightShadowCoord[i].w;
        // color = vec4(vec3(depth), 1);
        float texture_depth = textureProj(directional_light_shadow_maps[i], fDirectionalLightShadowCoord[i]).r;
        // color = vec4(vec3(texture_depth), 1);
        if (in_light(texture_depth, depth)) {
            color = vec4(1,0,1,1);
        }
    }
}
