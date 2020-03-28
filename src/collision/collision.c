/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

vec3 *random_points(float radius, int n)
{
    // So the "random" convex polyhedra have some sort of variety, use Gram-Schmidt to create a semi-random orthonormal basis, and
    // then have r1,r2,r3 be the "random principal axes", that weight the points in each of those directions.
    vec3 e1, e2, e3;
    while (vec3_dot(e1,e1) <= 0.01) e1 = new_vec3(frand()-0.5, frand()-0.5, frand()-0.5);
    e1 = vec3_normalize(e1);
    while (vec3_dot(e2,e2) <= 0.01) e2 = new_vec3(frand()-0.5, frand()-0.5, frand()-0.5);
    e2 = vec3_normalize(vec3_sub(e2, vec3_mul(e1, vec3_dot(e2, e1))));
    e3 = vec3_cross(e1, e2);
    // Extend these so the random convex polyhedra is roughly ellipsoidal.
    e1 = vec3_mul(e1, 0.5 + frand());
    e2 = vec3_mul(e1, 0.5 + frand());
    e3 = vec3_mul(e1, 0.5 + frand());

    vec3 *points = malloc(sizeof(vec3) * n);
    mem_check(points);
    for (int i = 0; i < n; i++) {
        points[i] = vec3_add(vec3_add(vec3_mul(e1, frand()), vec3_mul(e2, frand())), vec3_mul(e3, frand()));
    }
}
Geometry random_convex_polyhedra(float radius, int n)
{
    vec3 *points = random_points(radius, n);
    // MeshData mesh;
    // mesh.num_vertices = n;
    // mesh.vertex_format = VERTEX_FORMAT_3NU;
    // mesh.attribute_data[Position] = (float *) vertices;
}

vec3 *points;
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
    points = random_points(50, 20);
}
extern void loop_program(void)
{
    
}
extern void close_program(void)
{
}
