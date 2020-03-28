/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"


// Data structure for polyhedra, with adjacency information.
// This is not designed for compactness or direct efficiency, but rather flexibility as a data structure.
// Vertices do not contain adjacency information.
struct Polyhedra_s;
struct PolyhedraPoint_s;
struct PolyhedraEdge_s;
struct PolyhedraTriangle_s;

// A polyhedron feature can be cast to this, to allow generic doubly-linked list functions.
typedef struct DLNode_s {
    struct DLNode_s *prev;
    struct DLNode_s *next;
} DLNode;
typedef struct DLList_s {
    DLNode_s *first;
    DLNode_s *last;
} DLList;


// void *___polyhedron_add(size_t feature_type_size, Polyhedron *polyhedron, void *feature)
// {
//     polyhedron
// }
// #define polyhedron_insert(TYPE,POLYHEDRON,FEATURE_POINTER)\
//     (Polyhedron ## Type *) ___polyhedron_insert(sizeof(Polyhedron ## TYPE), POLYHEDRON, FEATURE_POINTER)


typedef struct PolyhedronEdge_s {
    struct PolyhedronEdge_s *prev;
    struct PolyhedronEdge_s *next;
    struct PolyhedronPoint_s *a;
    struct PolyhedronPoint_s *b;
    struct PolyhedronTriangle_s *left_triangle; // left of upwards arrow a->b.
    struct PolyhedronTriangle_s *right_triangle; // right of upwards arrow a->b.
} PolyhedronEdge;
typedef struct PolyhedronPoint_s {
    struct PolyhedronPoint_s *prev;
    struct PolyhedronPoint_s *next;
    vec3 position;
} PolyhedronPoint_s;
typedef struct PolyhedronTriangle_s {
    struct PolyhedronTriangle_s *prev;
    struct PolyhedronTriangle_s *next;
    struct PolyhedronPoint_s *points[3];
    // Adjacent triangles.
    struct PolyhedronTriangle_s *adj[3]; //ab, bc, ca.
} PolyhedronTriangle;
typedef struct Polyhedron_s {
    DLList points;
    DLList edges;
    DLList triangles;
} Polyhedron;

Polyhedron new_polyhedron(void)
{
    Polyhedron poly = {0};
    return poly;
}

DLNode *dl_add(DLList *list, DLNode *node)
{
    if (list->first == NULL) {
        list->first = node;
        list->last = node;
    }
    node->prev = list->last;
    node->next = NULL;
    list->last.next = node;
    list->last = node;
}
void dl_remove(DLList *list, DLNode *node)
{
    if (node->prev == NULL) {
        list->first = node->next;
    }
    free(node);
}

PolyhedronPoint *polyhedron_add_point(Polyhedron *polyhedron, vec3 point)
{
    if (polyhedron->points.first == NULL) {
        polyhedron->points.first = polyhedron
    }
    PolyhedronPoint *new_point = malloc(sizeof(PolyhedronPoint));
    mem_check(new_point);
    new_point->position = point;
    polyhedron->points.last.next = new_point;
    polyhedron->points.last = new_point;
}

void convex_hull(vec3 *points, int num_points)
{
    // Start up a polyhedron data structure as a tetrahedron.
    if (num_points < 4) {
        // The points are their own convex hull.
        return;
    }
    Polyhedron poly = new_polyhedron();
    for (int i = 0; i < 4; i++) polyhedron_add_point(poly, points[i]);
}

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
Geometry random_convex_polyhedra(float radius, int n)
{
    vec3 *points = random_points(radius, n);
    // MeshData mesh;
    // mesh.num_vertices = n;
    // mesh.vertex_format = VERTEX_FORMAT_3NU;
    // mesh.attribute_data[Position] = (float *) vertices;
}

vec3 *points;
int num_points;
extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS && key == GLFW_KEY_P) {
        points = random_points(50, num_points);
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
    num_points = 20;
    points = random_points(50, num_points);
}
extern void loop_program(void)
{
    paint_points_c(Canvas3D, points, num_points, "b", 5);
}
extern void close_program(void)
{
}
