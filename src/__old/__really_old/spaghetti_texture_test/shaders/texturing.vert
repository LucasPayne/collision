#version 420

//-----make #include stuff work and synchronize vertex attributes to the vertex format bitmask positions
// through a standard header.
#define LOCATION_POSITION 0
#define LOCATION_COLOR 1
#define LOCATION_NORMAL 2
#define LOCATION_TEXCOORD 3

uniform float time;

layout (location = 0) in vec2 vPosition;
layout (location = 3) in vec2 vTexCoord;

out VertexOut {
    vec2 fTexCoord;
    vec2 fScreenPos;
};

void main(void)
{
    gl_Position = vec4(vPosition, 0, 1);
    fTexCoord = vTexCoord + vec2(sin(time)*0.3,cos(time)*0.3);
    fScreenPos = vPosition;
}
