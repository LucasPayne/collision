#version 420

// these model and projection matrices are actually for
// positions relative to a coordinate space which is viewed,
// showing 2d projective transformations.

// Differences between mat3 and mat3x3 etc.?
uniform mat3 model_matrix;
uniform mat3 projection_matrix;
uniform float aspect_ratio;
uniform mat4 camera_matrix;

in layout(location = 0) vec2 vPosition;

out vec4 fColor;
out vec4 fPosition;

void main(void)
{
    vec3 pos_2d_projection = vec3(vPosition, 1.0);
    pos_2d_projection *= model_matrix;
    pos_2d_projection *= projection_matrix;
    pos_2d_projection.x *= aspect_ratio;
    
    vec4 pos = vec4(pos_2d_projection.x/pos_2d_projection.z,
                    pos_2d_projection.y/pos_2d_projection.z,
                    1.0, 1.0);
    pos *= camera_matrix;
    gl_Position = vec4(pos.x, pos.y, pos.z, pos.z);


    fColor = vec4(1.0, 0.0, 0.0, 1.0);

    fPosition = gl_Position;
}
