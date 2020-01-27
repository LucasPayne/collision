/*--------------------------------------------------------------------------------
    render_shadow_map: fragment shader
This shader is for debugging shadow maps, or other depth maps, allowing them to be
rendered to quads.
https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
--------------------------------------------------------------------------------*/

uniform sampler2D shadow_map;

in vOut {
    vec2 fTexCoord;
};
out vec4 color;

void main(void)
{
    float depth = texture(shadow_map, fTexCoord).r;
    color = vec4(vec3(depth), 1);
}

