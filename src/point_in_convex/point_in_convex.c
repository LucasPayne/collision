/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"
Polyhedron poly;

void create(void)
{
    poly = random_convex_polyhedron(100, 100);
    PolyhedronPoint *p = poly.points.first;
    float o = 70;
    vec3 shift = new_vec3(o*frand()-o/2,o*frand()-o/2,o*frand()-o/2);
    while (p != NULL) {
        p->position = vec3_add(p->position, shift);
        p = p->next;
    }
}

static void show_simplex(int n, vec3 simplex[], char *color_str)
{
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < i; j++) {
            paint_line_cv(Canvas3D, simplex[i], simplex[j], color_str, 10);
        }
    }
}

int N = 0;
void contains_origin(Polyhedron poly)
{
    int counter = 0;
#define check() if (N == counter++)
    vec3 simplex[4];
    int n = 2;
    simplex[0] = polyhedron_extreme_point(poly, new_vec3(1,1,1));
    simplex[1] = polyhedron_extreme_point(poly, vec3_neg(simplex[0]));
    paint_line_cv(Canvas3D, simplex[0], simplex[1], "k", 10);
    check() {
        return;
    }
    vec3 origin = vec3_zero();
    while (1) {
        show_simplex(n, simplex, "k");
        vec3 c = closest_point_on_simplex(n, simplex, origin);
        vec3 dir = vec3_neg(c);
        int replacing_index;
        if (n == 4) {
            if (point_in_tetrahedron(simplex[0],simplex[1],simplex[2],simplex[3], origin)) {
                paint_points_c(Canvas3D, &origin, 1, "tg", 50);
                return;
            }
            replacing_index = simplex_extreme_index(n, simplex, c);
            n --;
            check() {
                paint_points_c(Canvas3D, &simplex[replacing_index], 1, "y", 20);
                return;
            }
        } else replacing_index = n;
        check() {
            paint_points_c(Canvas3D, &c, 1, "b", 20);
            return;
        }
        vec3 new_point = polyhedron_extreme_point(poly, dir);
        check() {
            paint_points_c(Canvas3D, &new_point, 1, "b", 25);
            return;
        }
        simplex[replacing_index] = new_point;
        n ++;

        show_simplex(n, simplex, "k");
        check() return;

    }
}


extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_R) {
            create();
        }
        if (key == GLFW_KEY_T) N ++;
        if (key == GLFW_KEY_O) N = 0;
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
    create_key_camera_man(0,0,0,  0,0,0);
    create();
}
extern void loop_program(void)
{
    vec3 origin = vec3_zero();
    paint_points_c(Canvas3D, &origin, 1, "p", 25);
    draw_polyhedron(&poly, NULL);
    contains_origin(poly);
}
extern void close_program(void)
{
}
