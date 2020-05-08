/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

typedef struct StraightModel_s {
    float plane_height;
    float width;
    float height;
    int grid_tess_x;
    int grid_tess_y;
} StraightModel;
typedef struct StraightModel_s SM;

vec3 perspective_point(Logic *g, vec3 p);
void draw_projected_segment(Logic *g, vec3 a, vec3 b, vec4 color, vec4 projected_color, float width);
StraightModel *StraightModel_add(EntityID e, float width, float height, float plane_height);

void StraightModel_mouse_button_listener(Logic *logic, int button, int action, int mods)
{
    printf("Wow!\n");
}

extern void input_event(int key, int action, int mods)
{
}
extern void mouse_button_event(MouseButton button, bool click, float x, float y)
{
}
extern void mouse_position_event(double x, double y)
{
}
extern void mouse_move_event(double dx, double dy)
{
}
extern void init_program(void)
{
    // open_scene(g_scenes, "block_on_floor");

    // create_key_camera_man(0,0,0,  0,0,0);

    {
    EntityID e = new_gameobject(0,0,0, 0,0,0, true);
    SM *sm = StraightModel_add(e, 100, 100, 20);
    Logic_add_input(get_aspect_type(e, Logic), INPUT_MOUSE_BUTTON, StraightModel_mouse_button_listener);
    }

    {
    EntityID e = new_gameobject(-100,0,0, 0,0,0, true);
    SM *sm = StraightModel_add(e, 100, 100, 20);
    }
    //---for some reason if this is the first entity created, nothing can be seen.
    Player_create_default(0,0,0, 0,0);
}

vec3 perspective_point(Logic *g, vec3 p)
{
    StraightModel *sm = g->data;
    float z = p.vals[1];
    const float epsilon = 0.0004;
    if (z < epsilon) return new_vec3(100000,100000,100000);
    return new_vec3(sm->plane_height * p.vals[0] / z, sm->plane_height, sm->plane_height * p.vals[2] / z);
}

void draw_projected_segment(Logic *g, vec3 a, vec3 b, vec4 color, vec4 projected_color, float width)
{
    StraightModel *sm = g->data;
    paint_line_v(Canvas3D, a, b, color, width);
    vec3 ap = perspective_point(g, a);
    vec3 bp = perspective_point(g, b);
    paint_line_v(Canvas3D, a, b, color, width);
    // paint_triangle_v(Canvas3D, new_vec3(0,0,0), ap, bp, projected_color);
    paint_line_v(Canvas3D, ap, bp, projected_color, width);
    // paint_quad_v(Canvas3D, ap, bp, b, a, color);
    paint_line_cv(Canvas3D, vec3_zero(), a, "tk", 1);
    paint_line_v(Canvas3D, vec3_zero(), b, new_vec4(0,0.2,0.13,0.4), 1);
}

void StraightModel_update(Logic *g)
{
    StraightModel *sm = g->data;
    Transform *t = Transform_get_a(g);
    painting_matrix(Transform_get_matrix_a(g));

    vec3 base[4] = {
        {{-sm->width/2,0,-sm->height/2}},
        {{sm->width/2,0,-sm->height/2}},
        {{sm->width/2,0,sm->height/2}},
        {{-sm->width/2,0,sm->height/2}},
    };
    vec3 plane[4];
    for (int i = 0; i < 4; i++) plane[i] = base[i], plane[i].vals[1] += sm->plane_height;
    paint_sphere_v(Canvas3D, new_vec3(0,0,0), 1, new_vec4(0,0,0,1));
    // paint_sphere_v(Canvas3D, plane_origin, 1, new_vec4(0,0.5,0,1));
    // paint_line_v(Canvas3D, origin, plane_origin, new_vec4(0,0,0,1), 0.8);
    paint_grid_v(Canvas3D, base[0], base[1], base[2], base[3], new_vec4(0,0.3,0.9,0.3), sm->grid_tess_x, sm->grid_tess_y, 1);
    paint_grid_v(Canvas3D, plane[0], plane[1], plane[2], plane[3], new_vec4(0,0.8,0.21,0.63), sm->grid_tess_x, sm->grid_tess_y, 2);
    
    for (int i = 0; i < 20; i++) {
        float t = i*2*M_PI/20;
        float tp = (i+1)*2*M_PI/20;
        draw_projected_segment(g, new_vec3(30*cos(time+t), 40, 13*sin(2*t)), new_vec3(30*cos(time+tp), 40, 13*sin(2*tp)), new_vec4(0,0,0,0.8), new_vec4(1,0,0,0.8), 5);
    }
    painting_matrix_reset();
}
StraightModel *StraightModel_add(EntityID e, float width, float height, float plane_height)
{
    StraightModel *sm = add_logic(e, StraightModel_update, StraightModel)->data;
    sm->width = width;
    sm->height = height;
    sm->plane_height = plane_height;
    sm->grid_tess_x = 18;
    sm->grid_tess_y = 18;
    return sm;
}


extern void loop_program(void)
{
    // Camera *camera;
    // for_aspect(Camera, _camera)
    //     camera = _camera;
    //     break;
    // end_for_aspect()
    // mat4x4 vp_matrix = Camera_vp_matrix(camera);
    // vec4 vanishing_plane = matrix_vec4(mat4x4_transpose(vp_matrix), new_vec4(0,0,0,1));
    // print_vec4(vanishing_plane);
}
extern void close_program(void)
{
}
