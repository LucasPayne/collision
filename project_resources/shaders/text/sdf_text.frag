#version 420

uniform sampler2D sdf_texture;

in vOut {
    vec2 fTexCoord;
};

out vec4 color;

void main(void)
{
    color = texture(sdf_texture, fTexCoord).r < 0.5 ? vec4(1,0,1,1) : vec4(0,0,0,1);
    // color = vec4(1,0,1,1);

    // float val = texture(sdf_texture, fTexCoord).r;
    // color = vec4(0,0,0,0);
    // if (val < 0.5) color = vec4(1,1,1,1);
}
