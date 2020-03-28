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
    DLNode *first;
    DLNode *last;
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
} PolyhedronPoint;
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


#define dl_add(LIST,NODE)\
    ___dl_add(*((DLList *) (&( LIST ))), (DLNode *) ( NODE ))
DLNode *___dl_add(DLList list, DLNode *node)
{
    if (list.first == NULL) {
        list.first = node;
        list.last = node;
    }
    node->prev = list.last;
    node->next = NULL;
    list.last->next = node;
    list.last = node;
}
#define dl_remove(LIST,NODE)\
    ___dl_remove(*((DLList *) (&( LIST ))), (DLNode *) ( NODE ))
void ___dl_remove(DLList list, DLNode *node)
{
    if (node->prev == NULL) {
        list.first = node->next;
    } else if (node->next == NULL) {
        list.first = NULL;
        list.last = NULL;
    } else {
        node->prev->next = node->next;
    }
    free(node);
}

PolyhedronPoint *polyhedron_add_point(Polyhedron *polyhedron, vec3 point)
{
    PolyhedronPoint *p = calloc(1, sizeof(PolyhedronPoint));
    mem_check(p);
    p->position = point;
    dl_add(polyhedron->points, p);
}
// It is up to the user of the polyhedron structure to maintain the fact that this is really does represent a polyhedron.
PolyhedronEdge *polyhedron_add_edge(Polyhedron *polyhedron, PolyhedronPoint *p1, PolyhedronPoint *p2)
{
    PolyhedronEdge *e = calloc(1, sizeof(PolyhedronEdge));
    mem_check(e);
    e->a = p1;
    e->b = p2;
    dl_add(polyhedron->edges, e);
}
// Triangles are added through their edges, so these edges must actually form a triangle for this to make sense.
PolyhedronTriangle *polyhedron_add_triangle(Polyhedron *polyhedron, PolyhedronEdge *e1, PolyhedronEdge *e2, PolyhedronEdge *e3)
{
    PolyhedronTriangle *t = calloc(1, sizeof(PolyhedronTriangle));
    mem_check(t);
    t->points[0] = e1->a;
    t->points[1] = e2->a;
    t->points[2] = e3->a;
    if (e3->b == e1->a) {
        e1->left_triangle = t;
        e2->left_triangle = t;
        e3->left_triangle = t;
    } else {
        // Assume this new triangle is given in edge-order clock-wise on the polyhedra.
        e1->right_triangle = t;
        e2->right_triangle = t;
        e3->right_triangle = t;
    }
}

float tetrahedron_6_times_volume(vec3 a, vec3 b, vec3 c, vec3 d)
{
    float a1,a2,a3,a4;
    a1 = a.vals[0]; a2 = a.vals[1]; a3 = a.vals[2]; a4 = a.vals[3];
    float b1,b2,b3,b4;
    b1 = b.vals[0]; b2 = b.vals[1]; b3 = b.vals[2]; b4 = b.vals[3];
    float c1,c2,c3,c4;
    c1 = c.vals[0]; c2 = c.vals[1]; c3 = c.vals[2]; c4 = c.vals[3];
    float d1,d2,d3,d4;
    d1 = d.vals[0]; d2 = d.vals[1]; d3 = d.vals[2]; d4 = d.vals[3];

    return a1*(b2*(c3-c4) - b3*(c2-c4) + b4*(c2-c3))
         - a2*(b1*(c3-c4) - b3*(c1-c4) + b4*(c1-c3))
         + a3*(b1*(c2-c4) - b2*(c1-c4) + b4*(c1-c2))
         - a4*(b1*(c2-c3) - b2*(c1-c3) + b3*(c1-c2));

}

void convex_hull(vec3 *points, int num_points)
{
    // Start up a polyhedron data structure as a tetrahedron.
    if (num_points < 4) {
        // The points are their own convex hull.
        return;
    }
    Polyhedron poly = new_polyhedron();
    {
        PolyhedronPoint *tetrahedron_points[4];
        for (int i = 0; i < 4; i++) {
            tetrahedron_points[i] = polyhedron_add_point(&poly, points[i]);
        }
        bool negative = tetrahedron_6_times_volume(points[0], points[1], points[2], points[3]) < 0;
        if (negative) {
            PolyhedronPoint *temp[4];
            memcpy(temp, tetrahedron_points, sizeof(PolyhedronPoint *) * 4);
            for (int i = 0; i < 4; i++) tetrahedron_points[3-i] = temp[i];
        }

        PolyhedronEdge *e1 = polyhedron_add_edge(&poly, tetrahedron_points[0], tetrahedron_points[1]);
        PolyhedronEdge *e2 = polyhedron_add_edge(&poly, tetrahedron_points[1], tetrahedron_points[2]);
        PolyhedronEdge *e3 = polyhedron_add_edge(&poly, tetrahedron_points[2], tetrahedron_points[3]);
        polyhedron_add_triangle(&poly, e1, e2, e3);
    }
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
