/*--------------------------------------------------------------------------------
    Test the global test_texture.
--------------------------------------------------------------------------------*/
#version 420
#block Standard3D

in vOut {
    vec2 fTexCoord;
};
out vec4 color;

void main(void)
{
    color = texture(test_texture, fTexCoord);
}
