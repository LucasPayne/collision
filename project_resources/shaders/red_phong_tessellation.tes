/*--------------------------------------------------------------------------------
Testing Phong tesselation with just a simple flat red material.
Based on paper by Tamy Boubeker and Marc Alexa, Phong Tesselation in SIGGRAPH Asia '08
--------------------------------------------------------------------------------*/
#version 420
#block Standard3D
layout (triangles, equal_spacing, ccw) in;

in tesselationOut {
    vec3 tNormal;
    vec3 tPosition;
} vertex_inputs[];

void main(void)
{
    float t1 = gl_TessCoord[0];
    float t2 = gl_TessCoord[1];
    float t3 = gl_TessCoord[2];

    vec3 a = vertex_inputs[0].tPosition;
    vec3 b = vertex_inputs[1].tPosition;
    vec3 c = vertex_inputs[2].tPosition;
    vec3 na = normalize(vertex_inputs[0].tNormal);
    vec3 nb = normalize(vertex_inputs[1].tNormal);
    vec3 nc = normalize(vertex_inputs[2].tNormal);

    vec3 p = (t1*a + t2*b + t3*c) / (t1 + t2 + t3);
    vec3 ap = p - na * dot(p - a, na);
    vec3 bp = p - nb * dot(p - b, nb);
    vec3 cp = p - nc * dot(p - c, nc);

    gl_Position = mvp_matrix * vec4((t1*ap + t2*bp + t3*cp) / (t1 + t2 + t3), 1);
}
