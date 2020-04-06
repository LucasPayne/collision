/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

#define GRID_WIDTH 32
#define GRID_HEIGHT 24
vec3 grid[GRID_HEIGHT][GRID_WIDTH] = {0};
EntityID tracer;

// tl, bl, br, tr.
// Anti-clockwise corners of the imaging plane of the ray tracer.
vec3 points[4];
float distance = 100;
float half_width = 50;
bool controlling_tracer = false;
bool showing_ray = true;
int showing_ray_i = 0;
int showing_ray_j = 0;
EntityID camera_man;
Camera *camera;

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
    // tl, bl, br, tr.
    points[0] = vec3_add(plane_point, vec3_add(vec3_mul(Transform_right(t), -half_width), vec3_mul(Transform_up(t), (GRID_HEIGHT * 1.0 / GRID_WIDTH) * half_width)));
    points[1] = vec3_add(plane_point, vec3_add(vec3_mul(Transform_right(t), -half_width), vec3_mul(Transform_up(t), (GRID_HEIGHT * 1.0 / GRID_WIDTH) * -half_width)));
    points[2] = vec3_add(plane_point, vec3_add(vec3_mul(Transform_right(t), half_width), vec3_mul(Transform_up(t), (GRID_HEIGHT * 1.0 / GRID_WIDTH) * -half_width)));
    points[3] = vec3_add(plane_point, vec3_add(vec3_mul(Transform_right(t), half_width), vec3_mul(Transform_up(t), (GRID_HEIGHT * 1.0 / GRID_WIDTH) * half_width)));

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
            vec3 t = vec3_lerp(points[0], points[3], j * 1.0 / GRID_WIDTH);
            vec3 b = vec3_lerp(points[1], points[2], j * 1.0 / GRID_WIDTH);
            grid_points[i][j] = vec3_lerp(t, b, i * 1.0 / GRID_HEIGHT);
        }
    }

    for (int i = 0; i < GRID_HEIGHT; i++) {
        for (int j = 0; j < GRID_WIDTH; j++) {
            vec4 color = new_vec4(grid[i][j].vals[0], grid[i][j].vals[1], grid[i][j].vals[2], 1);
            paint_quad_v(Canvas3D, grid_points[i][j], grid_points[i+1][j], grid_points[i+1][j+1], grid_points[i][j+1], color);
        }
    }
    
    if (showing_ray) {
        vec3 t = vec3_lerp(points[1], points[0], (showing_ray_i + 0.5) * 1.0 / GRID_WIDTH);
        vec3 b = vec3_lerp(points[2], points[3], (showing_ray_i + 0.5) * 1.0 / GRID_WIDTH);
        vec3 ray_point = vec3_lerp(t, b, (showing_ray_j + 0.5) * 1.0 / GRID_HEIGHT);
        paint_line_v(Canvas3D, position, ray_point, new_vec4(0.8,1,0.356,0.532), 3);
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
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
        vec3 ray_origin, ray_direction;
        Camera_ray(camera, mouse_screen_x, mouse_screen_y, &ray_origin, &ray_direction);
        printf("%.2f %.2f\n", mouse_screen_x, mouse_screen_y);
        print_vec3(ray_origin);
        print_vec3(ray_direction);
        paint_line_cv(Canvas3D, ray_origin, vec3_add(ray_origin, vec3_mul(ray_direction, 20)), "g", 10);
        paint_points_c(Canvas3D, &ray_origin, 1, "tp", 30);
        
        float rect_x, rect_y;
        if (ray_rectangle_plane_coordinates(ray_origin, ray_direction, points[0], points[1], points[2], points[3], &rect_x, &rect_y)) {
            printf("%.2f %.2f\n", rect_x, rect_y);
        }
    }
}
extern void cursor_move_event(double x, double y)
{
}
extern void init_program(void)
{
    camera_man = create_key_camera_man(0,0,0,  0,0,0);
    camera = get_aspect_type(camera_man, Camera);

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
