#version 420

in vOut {
    vec2 fTexCoord;
};

uniform sampler2D diffuse_map;

out vec4 color;
void main(void)
{
    vec4 val = texture(diffuse_map, fTexCoord);
    if (val.a < 0.1) discard;
    color = vec4(val.rgb, 1.0);
}

