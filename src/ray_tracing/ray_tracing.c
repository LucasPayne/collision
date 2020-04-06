/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

#define GRID_WIDTH 32
#define GRID_HEIGHT 24
vec3 grid[GRID_HEIGHT][GRID_WIDTH] = {0};
EntityID tracer;
float distance = 100;
float half_width = 50;
bool controlling_tracer = false;
EntityID camera_man;


void tracer_update(Logic *logic)
{
    Transform *t = get_sibling_aspect(logic, Transform);
    if (controlling_tracer) {
        float speed = 150;
        float move_x = 0, move_z = 0;
        if (alt_arrow_key_down(Right)) move_x -= speed * dt;
        if (alt_arrow_key_down(Left)) move_x += speed * dt;
        if (alt_arrow_key_down(Up)) move_z += speed * dt;
        if (alt_arrow_key_down(Down)) move_z -= speed * dt;
        Transform_move_relative(t, new_vec3(move_x, 0, move_z));

        float look_speed = 4;
        if (arrow_key_down(Left)) t->theta_y -= look_speed * dt;
        if (arrow_key_down(Right)) t->theta_y += look_speed * dt;
    }

    vec3 position = Transform_position(t);
    vec3 plane_point = vec3_add(position, vec3_mul(Transform_forward(t), distance));
    vec3 points[4];
    // tr, tl, bl, br
    points[0] = vec3_add(plane_point, vec3_add(vec3_mul(Transform_right(t), half_width), vec3_mul(Transform_up(t), (GRID_HEIGHT * 1.0 / GRID_WIDTH) * half_width)));
    points[1] = vec3_add(plane_point, vec3_add(vec3_mul(Transform_right(t), -half_width), vec3_mul(Transform_up(t), (GRID_HEIGHT * 1.0 / GRID_WIDTH) * half_width)));
    points[2] = vec3_add(plane_point, vec3_add(vec3_mul(Transform_right(t), -half_width), vec3_mul(Transform_up(t), (GRID_HEIGHT * 1.0 / GRID_WIDTH) * -half_width)));
    points[3] = vec3_add(plane_point, vec3_add(vec3_mul(Transform_right(t), half_width), vec3_mul(Transform_up(t), (GRID_HEIGHT * 1.0 / GRID_WIDTH) * -half_width)));

    paint_points_c(Canvas3D, &position, 1, "k", 12);
    // paint_line_cv(Canvas3D, position, plane_point, "k", 5);
    for (int i = 0; i < 4; i++) {
        float thickness = 2;
        paint_line_cv(Canvas3D, position, points[i], "k", thickness);
        paint_line_cv(Canvas3D, points[i], points[(i+1)%4], "k", thickness);
    }

    vec3 grid_points[GRID_HEIGHT + 1][GRID_WIDTH + 1];
    for (int i = 0; i < GRID_HEIGHT + 1; i++) {
        for (int j = 0; j < GRID_WIDTH + 1; j++) {
            vec3 t = vec3_lerp(points[1], points[0], j * 1.0 / GRID_WIDTH);
            vec3 b = vec3_lerp(points[2], points[3], j * 1.0 / GRID_WIDTH);
            grid_points[i][j] = vec3_lerp(t, b, i * 1.0 / GRID_HEIGHT);
        }
    }

    for (int i = 0; i < GRID_HEIGHT; i++) {
        for (int j = 0; j < GRID_WIDTH; j++) {
            vec4 color = new_vec4(grid[i][j].vals[0], grid[i][j].vals[1], grid[i][j].vals[2], 1);
            paint_quad_v(Canvas3D, grid_points[i][j], grid_points[i+1][j], grid_points[i+1][j+1], grid_points[i][j+1], color);
        }
    }
}

extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_M) {
            controlling_tracer = !controlling_tracer;
            get_aspect_type(camera_man, Logic)->updating = !controlling_tracer;
        }
    }
}
extern void mouse_button_event(int button, int action, int mods)
{
}
extern void cursor_move_event(double x, double y)
{
}
extern void init_program(void)
{
    camera_man = create_key_camera_man(0,0,0,  0,0,0);

    tracer = new_entity(4);
    Transform *t = add_aspect(tracer, Transform);
    Transform_set(t, 0,0,0, 0,0,0);
    t->euler_controlled = true;
    Logic_init(add_aspect(tracer, Logic), tracer_update);

    for (int i = 0; i < GRID_HEIGHT; i++) {
        for (int j = 0; j < GRID_WIDTH; j++) {
            grid[i][j] = vec3_add(rand_vec3(1), new_vec3(0.5,0.5,0.5));
        }
    }
}
extern void loop_program(void)
{
}
extern void close_program(void)
{
}
