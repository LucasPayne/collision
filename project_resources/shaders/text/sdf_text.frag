#version 420

uniform sampler2D sdf_texture;

in vOut {
    vec2 fTexCoord;
};

out vec4 color;

void main(void)
{
    float outline_width = 0.083;

    vec4 border_color = vec4(0,0,0,1);
    vec4 text_color = vec4(1,1,1,1);
    vec4 background_color = vec4(0,1,0,1);

    float val = texture(sdf_texture, fTexCoord).r;
    color = val < 0.5 ? background_color : text_color;
    if (0.5 < val && val < 0.5 + outline_width) color = border_color;
}
