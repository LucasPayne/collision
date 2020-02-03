#version 420

#block Standard3D

uniform sampler2D tex;
in vOut {
    vec4 uvd;
};
out vec4 color;

void main(void)
{
    color = vec4(1,1,1,1) - textureProj(tex, uvd);
}
