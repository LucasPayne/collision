/*--------------------------------------------------------------------------------
project_libs:
    + Engine
    + geometry
--------------------------------------------------------------------------------*/
#include "Engine.h"
#include "geometry.h"

vec3 *random_points(float radius, int n)
{
    // So the "random" convex polyhedra have some sort of variety, use Gram-Schmidt to create a semi-random orthonormal basis, and
    // then have r1,r2,r3 be the "random principal axes", that weight the points in each of those directions.
    vec3 e1, e2, e3;
    e1 = new_vec3(frand()-0.5, frand()-0.5, frand()-0.5);
    e1 = vec3_normalize(e1);
    e2 = new_vec3(frand()-0.5, frand()-0.5, frand()-0.5);
    e2 = vec3_normalize(vec3_sub(e2, vec3_mul(e1, vec3_dot(e2, e1))));
    e3 = vec3_cross(e1, e2);
    // Extend these so the random convex polyhedra is roughly ellipsoidal.
    e1 = vec3_mul(e1, 0.5 + frand());
    e2 = vec3_mul(e2, 0.5 + frand());
    e3 = vec3_mul(e3, 0.5 + frand());

    vec3 *points = malloc(sizeof(vec3) * n);
    mem_check(points);
    for (int i = 0; i < n; i++) {
        points[i] = vec3_mul(vec3_add(vec3_add(vec3_mul(e1, frand()-0.5), vec3_mul(e2, frand()-0.5)), vec3_mul(e3, frand()-0.5)), radius);
        print_vec3(points[i]);
    }
    return points;
}

vec3 *points;
int num_points;
Polyhedron hull;

void create_object(void)
{
    points = random_points(50, num_points);
    //-------destroy the previous hull polyhedron.
    hull = convex_hull(points, num_points);


}


extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS && key == GLFW_KEY_P) {
        create_object();
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
    num_points = 100;
    create_object();
}
extern void loop_program(void)
{
    paint_points_c(Canvas3D, points, num_points, "b", 5);
    if (viewing) {
        draw_polyhedron(&hull);
        draw_polyhedron_winding_order(&hull, "k", 10);
    }
}
extern void close_program(void)
{
}
