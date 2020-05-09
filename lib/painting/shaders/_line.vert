#version 420
#block Standard3D

layout (location = 0) in vec4 vPosition;

out VERTEX_OUT {
    vec4 vertex_position;
} vertex_out;

void main(void)
{
    vertex_out.vertex_position = vPosition;

    // vec4 pos = mvp_matrix * vPosition;
    // gl_Position = pos / pos.w;
    // gl_Position = pos;
}
