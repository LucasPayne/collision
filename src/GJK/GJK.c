/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

static int support_index(vec3 *points, int num_points, vec3 direction)
{
    float d = vec3_dot(points[0], direction); // At least one point must be given.
    int index = 0;
    for (int i = 1; i < num_points; i++) {
        float new_d = vec3_dot(points[i], direction);
        if (new_d > d) {
            d = new_d;
            index = i;
        }
    }
    return index;
}
// cso: Configuration space obstacle, another name for the Minkowski difference of two sets.
static vec3 cso_support(vec3 *A, int A_len, vec3 *B, int B_len, vec3 direction, int *A_support, int *B_support)
{
    // Returns the support vector in the Minkowski difference, and also gives the indices of the points in A and B whose difference is that support vector.
    *A_support = support_index(A, direction);
    *B_support = support_index(A, vec3_neg(direction));
    return A[*A_support] - B[*B_support];
}
bool convex_hull_intersection(vec3 *A, int A_len, vec3 *B, int B_len)
{

}

Polyhedron polyA;
Polyhedron polyB;
Polyhedron mink;
void compute_minkowski_difference(Polyhedron A, Polyhedron B)
{
    int num_points = polyhedron_num_points(&A) * polyhedron_num_points(&B);
    vec3 *points = malloc(sizeof(float)*3 * num_points);
    mem_check(points);
    PolyhedronPoint *p = A.points.first;
    int i = 0;
    while (p != NULL) {
        int j = 0;
        PolyhedronPoint *q = B.points.first;
        while (q != NULL) {
            points[i + j*polyhedron_num_points(&A)] = vec3_sub(p->position, q->position);
            q = q->next;
            j++;
        }
        p = p->next;
        i++;
    }
    mink = convex_hull(points, num_points);
    free(points);
}
void create(void)
{
    polyA = random_convex_polyhedron(100, 20);
    PolyhedronPoint *p = polyA.points.first;
    float o = 130;
    vec3 shift = rand_vec3(o);
    while (p != NULL) {
        p->position = vec3_add(p->position, shift);
        p = p->next;
    }
    polyB = random_convex_polyhedron(100, 20);
    p = polyB.points.first;
    o = 130;
    shift = rand_vec3(o);
    while (p != NULL) {
        p->position = vec3_add(p->position, shift);
        p = p->next;
    }
    compute_minkowski_difference(polyA, polyB);
}

extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_C) {
            create();
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
    create_key_camera_man(0,0,0,  0,0,0);
    create();
}
extern void loop_program(void)
{
    draw_polyhedron2(&polyA, NULL, "tb", 5);
    draw_polyhedron2(&polyB, NULL, "tr", 5);
    draw_polyhedron2(&mink, NULL, "k", 10);
    vec3 origin = vec3_zero();
    paint_points_c(Canvas3D, &origin, 1, "p", 25);
}
extern void close_program(void)
{
}
