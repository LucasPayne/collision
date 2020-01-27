/*--------------------------------------------------------------------------------
    texture: fragment shader
Straightforward texturing in 3D, with nothing extra.
--------------------------------------------------------------------------------*/
#version 420

uniform sampler2D diffuse_map;

in vOut {
    vec2 fTexCoord;
};
out vec4 color;

void main(void)
{
    color = texture(diffuse_map, fTexCoord);
}

