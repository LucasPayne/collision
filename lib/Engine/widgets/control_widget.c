#include "Engine.h"

static const vec4 widget_axis_colors[] = {{{1,0,0,1}},{{0,1,0,1}},{{0,0,1,1}}};
static const vec3 widget_axes[] = {{{1,0,0}}, {{0,1,0}}, {{0,0,1}}};
void ControlWidget_update(Logic *g)
{
    ControlWidget *widget = g->data;
    Transform *t = Transform_get_a(g);
    Transform_painting_matrix(t);

    for (int i = 0; i < 3; i++) {
        paint_line_v(Canvas3D, vec3_zero(), vec3_mul(widget_axes[i], widget->size), new_vec4(UNPACK_VEC3(widget_axis_colors[i]), widget->alpha), 4);
    }
    
    for (int i = 0; i < 3; i++) {
        vec3 e1 = vec3_mul(widget_axes[i], widget->size);
        vec3 e2 = vec3_mul(widget_axes[(i+1)%3], widget->size);
        vec4 color = vec4_lerp(widget_axis_colors[i], widget_axis_colors[(i+1)%3], 0.5);
        W(color) = widget->alpha;
        vec3 points[4];
        points[0] = vec3_zero();
        float multiplier = widget->dragging && widget->dragging_plane == i ? 0.73 : 0.55;
        points[1] = vec3_mul(e1, multiplier);
        points[3] = vec3_mul(e2, multiplier);
        points[2] = vec3_add(points[1], points[3]);
        paint_quad_vv(Canvas3D, points, color);
    }
}

void ControlWidget_mouse_button_listener(Logic *g, MouseButton button, bool click, float x, float y)
{
    ControlWidget *widget = g->data;

    if (widget->dragging) {
        if (button == MouseLeft && !click) {
            widget->dragging = false;
        }
    } else {
        if (click && button == MouseLeft) {
            Transform *t = Transform_get_a(g);
            mat4x4 matrix = Transform_matrix(t);
            // When clicking, check a ray against three axis-aligned rectangles. If any are intersected, the closest one is
            // the one chosen.
	    vec3 ray_origin, ray_direction;
            Camera_ray(g_main_camera, x, y, &ray_origin, &ray_direction);
            int clicked = -1; 
            float min_distance = 0;
	    vec3 drag_start_point;
            for (int i = 0; i < 3; i++) {
                vec3 points[4];
                points[0] = vec3_zero();
                points[1] = vec3_mul(widget_axes[i], 0.55 * widget->size);
                points[3] = vec3_mul(widget_axes[(i+1)%3], 0.55 * widget->size);
                points[2] = vec3_add(points[1], points[3]);
                for (int i = 0; i < 4; i++) points[i] = mat4x4_vec3(matrix, points[i]);
	        vec3 intersection;
                if (!ray_rectangle_intersection(ray_origin, ray_direction, points[0], points[1], points[2], points[3], &intersection)) continue;

                float distance = vec3_dot(vec3_sub(intersection, ray_origin), vec3_sub(intersection, ray_origin)); // distance-squared.
                if (clicked == -1 || distance < min_distance) {
                    clicked = i;
                    min_distance = distance;
                    drag_start_point = intersection;
                    // paint_points_c(Canvas3D, &drag_start_point, 1, "r", 20);
                    // pause();
                }
            }
            if (clicked != -1) {
                widget->dragging = true;
                widget->dragging_plane = clicked;
                widget->drag_offset = vec3_sub(Transform_position(t), drag_start_point);
            }
        }
    }
}
void ControlWidget_mouse_move_listener(Logic *g, float x, float y, float dx, float dy)
{
    ControlWidget *widget = g->data;
    Transform *t = Transform_get_a(g);
    mat4x4 matrix = Transform_matrix(t);
    if (widget->dragging) {
        vec3 ray_origin, ray_direction;
        Camera_ray(g_main_camera, x, y, &ray_origin, &ray_direction);
        vec3 a = mat4x4_vec3(matrix, vec3_zero());
        vec3 b = mat4x4_vec3(matrix, widget_axes[widget->dragging_plane]);
        vec3 c = mat4x4_vec3(matrix, widget_axes[(widget->dragging_plane+1)%3]);
        vec3 intersection;
	if (ray_triangle_plane_intersection(ray_origin, ray_direction, a, b, c, &intersection)) {
             Transform_set_position(t, vec3_add(widget->drag_offset, intersection));
        }
    }
}

ControlWidget *ControlWidget_add(EntityID entity, float size)
{
    Logic *g = add_logic(entity, ControlWidget_update, ControlWidget);
    Logic_add_input(g, INPUT_MOUSE_BUTTON, ControlWidget_mouse_button_listener);
    Logic_add_input(g, INPUT_MOUSE_MOVE, ControlWidget_mouse_move_listener);
    ControlWidget *cw = g->data;
    cw->size = size;
    cw->alpha = 1.0;
    return cw;
}
