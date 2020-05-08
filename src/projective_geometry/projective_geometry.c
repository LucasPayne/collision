/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

extern void input_event(int key, int action, int mods)
{
}
extern void mouse_button_event(int button, int action, int mods)
{
}
extern void cursor_move_event(double x, double y)
{
}
extern void init_program(void)
{
    create_key_camera_man(0,0,0,  0,0,0);
}

#define plane_height 20
float width = 100;
float height = 100;
vec3 origin = {{0,0,0}};
vec3 plane_origin = {{0,plane_height,0}};

vec3 perspective_point(vec3 p)
{
    float z = p.vals[1];
    const float epsilon = 0.0004;
    if (z < epsilon) return new_vec3(100000,100000,100000);
    return new_vec3(plane_height * p.vals[0] / z, plane_height, plane_height * p.vals[2] / z);
}

void draw_projected_segment(vec3 a, vec3 b, vec4 color, vec4 projected_color, float width)
{
    paint_line_v(Canvas3D, vec3_add(a, origin), vec3_add(b, origin), color, width);
    vec3 ap = perspective_point(a);
    vec3 bp = perspective_point(b);
    paint_line_v(Canvas3D, vec3_add(ap, origin), vec3_add(bp, origin), projected_color, width);
    paint_triangle_v(Canvas3D, origin, vec3_add(ap, origin), vec3_add(bp, origin), projected_color);
    paint_quad_v(Canvas3D, vec3_add(ap, origin), vec3_add(bp, origin), vec3_add(b, origin), vec3_add(a, origin), color);
}

extern void loop_program(void)
{
    Camera *camera;
    for_aspect(Camera, _camera)
        camera = _camera;
        break;
    end_for_aspect()
    mat4x4 vp_matrix = Camera_vp_matrix(camera);
    vec4 vanishing_plane = matrix_vec4(mat4x4_transpose(vp_matrix), new_vec4(0,0,0,1));
    print_vec4(vanishing_plane);

    int grid_tess_x = 18;
    int grid_tess_y = 18;
    vec3 base[4] = {
        {{-width/2,0,-height/2}},
        {{width/2,0,-height/2}},
        {{width/2,0,height/2}},
        {{-width/2,0,height/2}},
    };
    vec3 plane[4];
    for (int i = 0; i < 4; i++) plane[i] = base[i], plane[i].vals[1] += plane_height;
    paint_sphere_v(Canvas3D, origin, 1, new_vec4(0,0,0,1));
    paint_sphere_v(Canvas3D, plane_origin, 1, new_vec4(0,0.5,0,1));
    paint_line_v(Canvas3D, origin, plane_origin, new_vec4(0,0,0,1), 0.8);
    paint_grid_v(Canvas3D, base[0], base[1], base[2], base[3], new_vec4(0,0.3,0.9,0.3), grid_tess_x, grid_tess_y, 1);
    paint_grid_v(Canvas3D, plane[0], plane[1], plane[2], plane[3], new_vec4(0,0.8,0.21,0.63), grid_tess_x, grid_tess_y, 2);
    
    for (int i = 0; i < 20; i++) {
        float t = i*2*M_PI/20;
        float tp = (i+1)*2*M_PI/20;
        draw_projected_segment(new_vec3(30*cos(time+t), 40, 13*sin(2*t)), new_vec3(30*cos(time+tp), 40, 13*sin(2*tp)), new_vec4(0,0,0,0.8), new_vec4(1,0,0,0.8), 2);
    }
}
extern void close_program(void)
{
}
