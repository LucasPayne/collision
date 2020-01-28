#version 420

in vOut {
    vec2 fTexCoord;
};
uniform sampler2D sprite;
out vec4 color;

void main(void)
{
    color = texture(sprite, fTexCoord);
}
