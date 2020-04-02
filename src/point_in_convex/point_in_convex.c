/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"
Polyhedron poly;

void create(void)
{
    poly = random_convex_polyhedron(100, 10);
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
            paint_line_cv(Canvas3D, simplex[i], simplex[j], color_str, 20);
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
    vec3 origin = vec3_zero();

    while (1) {
        printf("num points: %d\n", n);
        vec3 c = closest_point_on_simplex(n, simplex, origin);
        vec3 dir = vec3_neg(c);
        if (n == 4 && point_in_tetrahedron(simplex[0],simplex[1],simplex[2],simplex[3], origin)) {
            // The origin is inside.
            show_simplex(n, simplex, "g");
            paint_points_c(Canvas3D, &origin, 1, "tg", 50);
            return;
        }
        vec3 new_point = polyhedron_extreme_point(poly, dir);

        bool on_simplex = false;
        for (int i = 0; i < n; i++) {
            if (vec3_equal(simplex[i], new_point)) {
                on_simplex = true;
                break;
            }
        }
        if (n == 4 && !on_simplex) {
            int replace = simplex_extreme_index(n, simplex, c);
            simplex[replace] = new_point;
        } else if (n == 3 && on_simplex) {
            show_simplex(n, simplex, "r");
            paint_points_c(Canvas3D, &origin, 1, "tr", 50);
            vec3 closest_on_poly = closest_point_on_triangle_to_point(simplex[0], simplex[1], simplex[2], origin);
            paint_points_c(Canvas3D, &closest_on_poly, 1, "tb", 50);
            return;
        } else if (n == 2 && on_simplex) {
            show_simplex(n, simplex, "r");
            paint_points_c(Canvas3D, &origin, 1, "tr", 50);
            vec3 closest_on_poly = closest_point_on_line_segment_to_point(simplex[0], simplex[1], origin);
            paint_points_c(Canvas3D, &closest_on_poly, 1, "tb", 50);
        } else if (!on_simplex) {
            simplex[n++] = new_point;
        } else {
            int remove = simplex_extreme_index(n, simplex, c);
            for (int j = remove; j < n - 1; j++) {
                simplex[j] = simplex[j + 1];
            }
            n --;
        }

        check() {
            show_simplex(n, simplex, "k");
            return;
        }
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
