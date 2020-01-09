#version 420

uniform float time;
uniform float aspect_ratio;
uniform mat4x4 rotation_matrix;
uniform mat4x4 view_matrix;

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec4 vColor;
layout(location = 2) in vec2 texture_coord;
out vec4 fColor;
out vec2 ftexture_coord;

void main()
{
    float speed = 1;
    vec4 pos = vPosition;
#if 0
    pos.x += 0.4 * sin(time + pos.x);
    pos.y += 0.4 * sin(2.1 * time + 1.3 + pos.y);
    pos.z += 0.4 * sin(0.89 * time + 3.7777 + pos.z);
#endif
#if 0
    pos = vec4(cos(time*speed)*pos.x + sin(time*speed)*pos.z,
                       pos.y, 
                       -sin(time*speed)*pos.x + cos(time*speed)*pos.z,
                       1.0);
    float speed2 = 2.8;
    pos = vec4(pos.x,
                       cos(time*speed2)*pos.y + sin(time*speed2)*pos.z,
                       -sin(time*speed2)*pos.y + cos(time*speed2)*pos.z,
                       1.0);

    /* fColor = vec4(1.0, 1.0, 1.0, 1.0); */
    /* gl_Position = vPosition; */


    /* fColor = vec4(gl_Position.z * 2.2, */
    /*               gl_Position.z * 2.2, */
    /*               gl_Position.z * 2.2, */
    /*               1.0); */

    /* fColor = vec4(vPosition.x, vPosition.y, 0.0, 1.0); */
    /* fColor = vec4(vPosition.x, vPosition.y, vPosition.z, 1.0); */
#else
    // Model
    pos *= rotation_matrix;
    /* pos *= transpose(view_matrix); // eye space */
    // View
    pos *= view_matrix; // eye space
    // Per-renderthing the model and view matrix can be premultiplied.
#endif
    // "Projection"
    // "aspect ratio" here is height/width
    gl_Position = vec4(pos.x * aspect_ratio, pos.y, pos.z, pos.z);
    fColor = vColor;
    ftexture_coord = texture_coord;
}
