/*--------------------------------------------------------------------------------
Phong tesselation.
Based on paper by Tamy Boubeker and Marc Alexa, Phong Tesselation in SIGGRAPH Asia '08

This and the control shader are for a phong-tesselation extension of the textured_phong_shadows
material.
--------------------------------------------------------------------------------*/
#version 420
#block Standard3D
#block Lights

layout (triangles, equal_spacing, ccw) in;

in tesselationOut {
    vec2 tTexCoord;
    vec4 tPosition;
    vec3 tNormal;
    vec4 tDirectionalLightShadowCoord[MAX_NUM_DIRECTIONAL_LIGHTS * NUM_FRUSTUM_SEGMENTS];
} vertex_inputs[];

out vOut {
    vec2 fTexCoord;
    vec4 fPosition;
    vec3 fNormal;
    vec4 fDirectionalLightShadowCoord[MAX_NUM_DIRECTIONAL_LIGHTS * NUM_FRUSTUM_SEGMENTS];
};

void main(void)
{
    float t1 = gl_TessCoord[0];
    float t2 = gl_TessCoord[1];
    float t3 = gl_TessCoord[2];
    vec3 a = gl_in[0].gl_Position.xyz / gl_in[0].gl_Position.w;
    vec3 b = gl_in[1].gl_Position.xyz / gl_in[0].gl_Position.w;
    vec3 c = gl_in[2].gl_Position.xyz / gl_in[0].gl_Position.w;
    vec3 na = normalize(vertex_inputs[0].tNormal);
    vec3 nb = normalize(vertex_inputs[1].tNormal);
    vec3 nc = normalize(vertex_inputs[2].tNormal);

    vec3 p = (t1*a + t2*b + t3*c) / (t1 + t2 + t3);
    vec3 ap = p - na * dot(p - a, na);
    vec3 bp = p - nb * dot(p - b, nb);
    vec3 cp = p - nc * dot(p - c, nc);

    gl_Position = vec4((t1*ap + t2*bp + t3*cp) / (t1 + t2 + t3), 1);
}
