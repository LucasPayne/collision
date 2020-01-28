/*--------------------------------------------------------------------------------
Taken from https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping.
This makes the variance in a depth map of a projection rendering easier to see,
for debugging.
--------------------------------------------------------------------------------*/
#version 420

#block Standard3D

out vec4 FragColor;
  
in vOut {
    vec2 fTexCoord;
};

uniform sampler2D shadow_map;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()
{             
    float depthValue = texture(shadow_map, fTexCoord).r;
    FragColor = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0); // perspective
    // FragColor = vec4(vec3(depthValue), 1.0); // orthographic
} 
