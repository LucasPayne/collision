/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"


// Data structure for polyhedra, with adjacency information.
// This is not designed for compactness or direct efficiency, but rather flexibility as a data structure.
// Vertices do not contain adjacency information.
// Marks are available space for algorithms to mark triangles as traversed, etc.
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
    struct PolyhedronTriangle_s *triangles[2];
    int mark;
} PolyhedronEdge;
typedef struct PolyhedronPoint_s {
    struct PolyhedronPoint_s *prev;
    struct PolyhedronPoint_s *next;
    vec3 position;
    int mark;
} PolyhedronPoint;
typedef struct PolyhedronTriangle_s {
    struct PolyhedronTriangle_s *prev;
    struct PolyhedronTriangle_s *next;
    struct PolyhedronPoint_s *points[3];
    struct PolyhedronEdge_s *edges[3];
    // Adjacent triangles.
    // struct PolyhedronTriangle_s *adj[3]; //ab, bc, ca.
    int mark;
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
    ___dl_add((DLList *) ( LIST ), (DLNode *) ( NODE ))
DLNode *___dl_add(DLList *list, DLNode *node)
{
    if (list->first == NULL) {
        list->first = node;
        list->last = node;
        return node;
    }
    node->prev = list->last;
    node->next = NULL;
    list->last->next = node;
    list->last = node;
    return node;
}
#define dl_remove(LIST,NODE)\
    ___dl_remove((DLList *) ( LIST ), (DLNode *) ( NODE ))
void ___dl_remove(DLList *list, DLNode *node)
{
    if (node == list->first && node == list->last) {
        printf("removed all\n");
        list->first = NULL;
        list->last = NULL;
    } else if (node == list->first) {
        printf("removed first\n");
        list->first = list->first->next;
        list->first->prev = NULL;
    } else if (node == list->last) {
        printf("removed last\n");
        list->last = list->last->prev;
        list->last->next = NULL;
    } else {
        printf("removed middle\n");
        node->prev->next = node->next;
        node->next->prev = node->prev->next;
    }
    free(node);
}

PolyhedronPoint *polyhedron_add_point(Polyhedron *polyhedron, vec3 point)
{
    PolyhedronPoint *p = calloc(1, sizeof(PolyhedronPoint));
    mem_check(p);
    p->position = point;
    dl_add(&polyhedron->points, p);
}
// It is up to the user of the polyhedron structure to maintain the fact that this is really does represent a polyhedron.
PolyhedronEdge *polyhedron_add_edge(Polyhedron *polyhedron, PolyhedronPoint *p1, PolyhedronPoint *p2)
{
    PolyhedronEdge *e = calloc(1, sizeof(PolyhedronEdge));
    mem_check(e);
    e->a = p1;
    e->b = p2;
    dl_add(&polyhedron->edges, e);
}
// Triangles are added through their edges, so these edges must actually form a triangle for this to make sense.


PolyhedronTriangle *polyhedron_add_triangle(Polyhedron *polyhedron, PolyhedronPoint *a, PolyhedronPoint *b, PolyhedronPoint *c, PolyhedronEdge *e1, PolyhedronEdge *e2, PolyhedronEdge *e3)
{
    PolyhedronTriangle *t = calloc(1, sizeof(PolyhedronTriangle));
    mem_check(t);
    // Get three unequal points from the edges (this complication is because in the polyhedron structure, feature ordering does not matter, only adjacency).
    // t->points[0] = e1->a;
    // t->points[1] = e2->a != e1->a ? e2->a : e2->b;
    // t->points[2] = e3->a != e1->a && e3->a != e2->a ? e3->a : e3->b;
    t->points[0] = a;
    t->points[1] = b;
    t->points[2] = c;
    // Add the triangle to the empty triangle slots of each edge.
    // if (e1->triangles[0] != NULL) t->adj[0] = e1->triangles[0];
    // if (e1->triangles[1] != NULL) t->adj[0] = e1->triangles[1];
    // if (e2->triangles[0] != NULL) t->adj[1] = e2->triangles[0];
    // if (e2->triangles[1] != NULL) t->adj[1] = e2->triangles[1];
    // if (e3->triangles[0] != NULL) t->adj[2] = e3->triangles[0];
    // if (e3->triangles[1] != NULL) t->adj[2] = e3->triangles[1];
    t->edges[0] = e1;
    t->edges[1] = e2;
    t->edges[2] = e3;

    e1->triangles[e1->triangles[0] == NULL ? 0 : 1] = t;
    e2->triangles[e2->triangles[0] == NULL ? 0 : 1] = t;
    e3->triangles[e3->triangles[0] == NULL ? 0 : 1] = t;
    dl_add(&polyhedron->triangles, t);
}
void polyhedron_remove_point(Polyhedron *poly, PolyhedronPoint *p)
{
    dl_remove(&poly->points, p);
}
void polyhedron_remove_edge(Polyhedron *poly, PolyhedronEdge *e)
{
    dl_remove(&poly->edges, e);
}
void polyhedron_remove_triangle(Polyhedron *poly, PolyhedronTriangle *t)
{
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            if (t->edges[i]->triangles[j] == t) t->edges[i]->triangles[j] = NULL;
        }
    }
    dl_remove(&poly->triangles, t);
}

float tetrahedron_6_times_volume(vec3 a, vec3 b, vec3 c, vec3 d)
{
    float a1,a2,a3,a4;
    a1 = a.vals[0]; a2 = b.vals[0]; a3 = c.vals[0]; a4 = d.vals[0];
    float b1,b2,b3,b4;
    b1 = a.vals[1]; b2 = b.vals[1]; b3 = c.vals[1]; b4 = d.vals[1];
    float c1,c2,c3,c4;
    c1 = a.vals[2]; c2 = b.vals[2]; c3 = c.vals[2]; c4 = d.vals[2];
    float d1,d2,d3,d4;
    d1 = a.vals[3]; d2 = b.vals[3]; d3 = c.vals[3]; d4 = d.vals[3];

    return a1*(b2*(c3-c4) - b3*(c2-c4) + b4*(c2-c3))
         - a2*(b1*(c3-c4) - b3*(c1-c4) + b4*(c1-c3))
         + a3*(b1*(c2-c4) - b2*(c1-c4) + b4*(c1-c2))
         - a4*(b1*(c2-c3) - b2*(c1-c3) + b3*(c1-c2));

}

void print_polyhedron(Polyhedron *p)
{
    PolyhedronPoint *point = p->points.first;
    printf("Points\n");
    printf("--------------------------------------------------------------------------------\n");
    int num_points = 0;
    while (point != NULL) {
        point->mark = num_points; // give this point a number.
        num_points ++;
        point = point->next;
    }
    printf("%d points\n", num_points);
    printf("Edges\n");
    printf("--------------------------------------------------------------------------------\n");
    PolyhedronEdge *edge = p->edges.first;
    int num_edges = 0;
    while (edge != NULL) {
        printf("e%d: %d->%d\n", num_edges, edge->a->mark, edge->b->mark);
        edge->mark = num_edges;
        num_edges ++;
        edge = edge->next;
    }
    printf("Triangles\n");
    printf("--------------------------------------------------------------------------------\n");
    PolyhedronTriangle *t = p->triangles.first;
    int num_triangles = 0;
    while (t != NULL) {
        printf("t%d : %d, %d, %d\n", num_triangles, t->points[0]->mark, t->points[1]->mark, t->points[2]->mark);
        t->mark = num_triangles;
        num_triangles ++;
        t = t->next;
    }
}
void draw_polyhedron(Polyhedron *p)
{
    PolyhedronPoint *point = p->points.first;
    while (point != NULL) {
        paint_points_c(Canvas3D, &point->position, 1, "r", 20);
        point = point->next;
    }
    PolyhedronEdge *edge = p->edges.first;
    while (edge != NULL) {
        paint_line_cv(Canvas3D, edge->a->position, edge->b->position, "r", 10);
        edge = edge->next;
    }
    PolyhedronTriangle *t = p->triangles.first;
    while (t != NULL) {
        paint_triangle_cv(Canvas3D, t->points[0]->position, t->points[1]->position, t->points[2]->position, "tg");
        t = t->next;
    }
}



// Definitions of marks this algorithm makes on features, during processing.
#define VISIBLE true
#define INVISIBLE false
#define NEEDED 1 << 0
#define BOUNDARY 1 << 1
void convex_hull(vec3 *points, int num_points)
{
#define DEBUG true
    if (num_points < 4) {
        // The points are their own convex hull.
        return;
    }
    // Start up a polyhedron data structure as a tetrahedron.
    Polyhedron poly = new_polyhedron();
    {
        PolyhedronPoint *tetrahedron_points[4];
        for (int i = 0; i < 4; i++) {
            tetrahedron_points[i] = polyhedron_add_point(&poly, points[i]);
        }
        bool negative = tetrahedron_6_times_volume(points[0], points[1], points[2], points[3]) < 0;
        if (negative) {
            // Fix the orientation by swapping two of the points.
            PolyhedronPoint *temp = tetrahedron_points[0];
            tetrahedron_points[0] = tetrahedron_points[1];
            tetrahedron_points[1] = temp;
        }
        PolyhedronEdge *e1 = polyhedron_add_edge(&poly, tetrahedron_points[0], tetrahedron_points[1]);
        PolyhedronEdge *e2 = polyhedron_add_edge(&poly, tetrahedron_points[1], tetrahedron_points[2]);
        PolyhedronEdge *e3 = polyhedron_add_edge(&poly, tetrahedron_points[2], tetrahedron_points[0]);
        polyhedron_add_triangle(&poly, tetrahedron_points[0], tetrahedron_points[1], tetrahedron_points[2], e1, e2, e3);
        PolyhedronEdge *e4 = polyhedron_add_edge(&poly, tetrahedron_points[0], tetrahedron_points[3]);
        PolyhedronEdge *e5 = polyhedron_add_edge(&poly, tetrahedron_points[1], tetrahedron_points[3]);
        PolyhedronEdge *e6 = polyhedron_add_edge(&poly, tetrahedron_points[2], tetrahedron_points[3]);
        polyhedron_add_triangle(&poly, tetrahedron_points[3], tetrahedron_points[1], tetrahedron_points[0], e1, e5, e4);
        polyhedron_add_triangle(&poly, tetrahedron_points[3], tetrahedron_points[2], tetrahedron_points[1], e2, e6, e5);
        polyhedron_add_triangle(&poly, tetrahedron_points[3], tetrahedron_points[0], tetrahedron_points[2], e3, e4, e6);
    }
    #if DEBUG
        printf("Made tetrahedron.\n");
        print_polyhedron(&poly);
        getchar();
    #endif
    for (int i = 4; i < num_points; i++) {
        #if DEBUG
            print_polyhedron(&poly);
            printf("Adding point %d...\n", i);
        #endif
        // Clean up the marks for points and edges (not neccessary for triangles, since they are always set).
        PolyhedronPoint *p = poly.points.first;
        while (p != NULL) { p->mark = 0; p = p->next; }
        PolyhedronEdge *e = poly.edges.first;
        while (e != NULL) { e->mark = 0; e = e->next; }
        #if DEBUG
            printf("Cleaned up marks.\n");
            getchar();
        #endif
        // For each triangle, mark as visible or invisible from the point of view of the new point.
        // If none are visible, then this point is in the hull so far, so continue.
        // Otherwise, use the marks scratched during the triangle checks to remove all visible triangles and
        // all other uneccessary features.
        // Then, add a new triangle for each border edge, which is an edge whose adjacent triangles have opposing visibility/invisibility.
        bool any_visible = false;
        PolyhedronTriangle *t = poly.triangles.first;
        while (t != NULL) {
            float v = tetrahedron_6_times_volume(t->points[0]->position, t->points[1]->position, t->points[2]->position, points[i]);
            if (v < 0) {
                t->mark = VISIBLE;
                any_visible = true;
            } else {
                t->mark = INVISIBLE;
                // mark edges and vertices of this triangle as necessary (they won't be deleted).
                t->points[0]->mark |= NEEDED;
                t->points[1]->mark |= NEEDED;
                t->points[2]->mark |= NEEDED;
                t->edges[0]->mark |= NEEDED;
                t->edges[1]->mark |= NEEDED;
                t->edges[2]->mark |= NEEDED;
            }
            t = t->next;
        }
        if (!any_visible) {
            #if DEBUG
                printf("None are visible.");
                getchar();
            #endif
            continue; // The point is in the hull so far.
        }
        #if DEBUG
            printf("Marked visibility.");
            getchar();
        #endif

        // Before deleting anything, use the visibility information to mark the borders. Then when triangles are deleted, the borders are still known.
        // This could be done more cleanly without looping over features so many times.
        e = poly.edges.first;
        while (e != NULL) {
            if (e->triangles[0]->mark != e->triangles[1]->mark) {
                e->mark |= BOUNDARY;
            }
            e = e->next;
        }
        // Remove all visible triangles (that weren't just added), and unneeded points and edges.
        printf("Removing triangles...\n");
        getchar();
        t = poly.triangles.first;
        while (t != NULL) {
            if (t->mark == VISIBLE) {
                PolyhedronTriangle *to_remove = t;
                t = t->next;
                polyhedron_remove_triangle(&poly, to_remove);
                continue;
            }
            t = t->next;
        }
        printf("Removed triangles.\n");
        printf("Removing points...\n");
        getchar();
        p = poly.points.first;
        while (p != NULL) {
            if ((p->mark & NEEDED) == 0) {
                PolyhedronPoint *to_remove = p;
                p = p->next;
                polyhedron_remove_point(&poly, to_remove);
                continue;
            }
            p = p->next;
        }
        printf("Removing edges...\n");
        getchar();
        e = poly.edges.first;
        while (e != NULL) {
            if ((e->mark & NEEDED) == 0) {
                PolyhedronEdge *to_remove = e;
                e = e->next;
                polyhedron_remove_edge(&poly, to_remove);
                continue;
            }
            e = e->next;
        }
        printf("Adding the cone.\n");
        getchar();
        // Add the new point, and new edges and triangles to create a cone from the new point to the border.
        PolyhedronPoint *new_point = polyhedron_add_point(&poly, points[i]);
        e = poly.edges.first;
        while (e != NULL) {
            if ((e->mark & BOUNDARY) != 0) {
                bool reverse = false;
                for (int i = 0; i < 2; i++) {
                    if (e->triangles[i] != NULL) {
                        for (int j = 0; j < 3; j++) {
                            if (e->a == e->triangles[i]->points[j] && e->b == e->triangles[i]->points[(j+1)%3]) reverse = true;
                        }
                        break;
                    }
                }
                PolyhedronEdge *e1 = polyhedron_add_edge(&poly, new_point, e->a);
                PolyhedronEdge *e2 = polyhedron_add_edge(&poly, new_point, e->b);
                PolyhedronTriangle *new_triangle = polyhedron_add_triangle(&poly, new_point, reverse ? e->b : e->a, reverse ? e->a : e->b, e1, e, e2);
            }
            e = e->next;
        }
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
Polyhedron poly;
int adding_point;

static void draw_polyhedron_winding_order(Polyhedron *poly, char *color_str, float line_width)
{
    PolyhedronTriangle *t = poly->triangles.first;
    while (t != NULL) {
        vec3 ps[3];
        for (int i = 0; i < 3; i++) {
            float alpha = 0.1;
            ps[i] = triangle_blend(t->points[(i+1)%3]->position, t->points[(i+2)%3]->position, t->points[i]->position,
                                   alpha, alpha, 1-2*alpha);
        }
        for (int i = 0; i < 2; i++) {
            paint_line_cv(Canvas3D, ps[i], ps[i+1], color_str, line_width);
        }
        vec3 end = vec3_lerp(ps[2], ps[0], 0.8);
        paint_line_cv(Canvas3D, ps[2], end, color_str, line_width);
        t = t->next;
    }
}

static void restart_poly(void)
{
    adding_point = 0;
    poly = new_polyhedron();
    {
        PolyhedronPoint *tetrahedron_points[4];
        for (int i = 0; i < 4; i++) {
            tetrahedron_points[i] = polyhedron_add_point(&poly, points[i]);
        }
        bool negative = tetrahedron_6_times_volume(points[0], points[1], points[2], points[3]) < 0;
        if (negative) {
            // Fix the orientation by swapping two of the points.
            PolyhedronPoint *temp = tetrahedron_points[0];
            tetrahedron_points[0] = tetrahedron_points[1];
            tetrahedron_points[1] = temp;
        }
        PolyhedronEdge *e1 = polyhedron_add_edge(&poly, tetrahedron_points[0], tetrahedron_points[1]);
        PolyhedronEdge *e2 = polyhedron_add_edge(&poly, tetrahedron_points[1], tetrahedron_points[2]);
        PolyhedronEdge *e3 = polyhedron_add_edge(&poly, tetrahedron_points[2], tetrahedron_points[0]);
        polyhedron_add_triangle(&poly, tetrahedron_points[0], tetrahedron_points[1], tetrahedron_points[2], e1, e2, e3);
        PolyhedronEdge *e4 = polyhedron_add_edge(&poly, tetrahedron_points[0], tetrahedron_points[3]);
        PolyhedronEdge *e5 = polyhedron_add_edge(&poly, tetrahedron_points[1], tetrahedron_points[3]);
        PolyhedronEdge *e6 = polyhedron_add_edge(&poly, tetrahedron_points[2], tetrahedron_points[3]);
        polyhedron_add_triangle(&poly, tetrahedron_points[3], tetrahedron_points[1], tetrahedron_points[0], e1, e5, e4);
        polyhedron_add_triangle(&poly, tetrahedron_points[3], tetrahedron_points[2], tetrahedron_points[1], e2, e6, e5);
        polyhedron_add_triangle(&poly, tetrahedron_points[3], tetrahedron_points[0], tetrahedron_points[2], e3, e4, e6);
        // print_polyhedron(&poly);
        // draw_polyhedron(&poly);
    }
}
static void view_in_poly(void)
{
    Camera *camera;
    for_aspect(Camera, _camera)
        camera = _camera;
        break;
    end_for_aspect()
    vec3 camera_pos = Transform_position(other_aspect(camera, Transform));

    // Clean up the marks for points and edges (not neccessary for triangles, since they are always set).
    PolyhedronPoint *p = poly.points.first;
    while (p != NULL) {
        p->mark = false;
        p = p->next;
    }
    PolyhedronEdge *e = poly.edges.first;
    while (e != NULL) {
        e->mark = false;
        e = e->next;
    }

    // For each triangle, mark as visible or invisible from the point of view of the new point.
    // If none are visible, then this point is in the hull so far, so continue.
    // Otherwise, use the marks scratched during the triangle checks to remove all visible triangles and
    // all other uneccessary features.
    // Then, add a new triangle for each border edge, which is an edge whose adjacent triangles have opposing visibility/invisibility.
    bool any_visible = false;
    PolyhedronTriangle *t = poly.triangles.first;
    int count = 0;
    while (t != NULL) {
        count += 1;
        //if (tetrahedron_6_times_volume(t->points[0]->position, t->points[1]->position, t->points[2]->position, points[adding_point]) > 0) {
        float v = tetrahedron_6_times_volume(t->points[0]->position, t->points[1]->position, t->points[2]->position, points[adding_point]);
        //printf("%d: %.2f\n", count, v);
        if (v < 0) {
            t->mark = true; // marked as visible.
            any_visible = true;
        } else {
            t->mark = false; // marked as invisible.
            // mark edges and vertices of this triangle as necessary (they won't be deleted).
            t->points[0]->mark = true;
            t->points[1]->mark = true;
            t->points[2]->mark = true;
            t->edges[0]->mark = true;
            t->edges[1]->mark = true;
            t->edges[2]->mark = true;
        }
        t = t->next;
    }
    //if (!any_visible) continue; // the point is in the hull so far.
    if (!any_visible) {
        printf("Ok\n");
    }
    if (!any_visible) paint_points_c(Canvas3D, &points[adding_point], 1, "y", 100);
}

extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS && key == GLFW_KEY_P) {
        points = random_points(50, num_points);
        restart_poly();
    }
    if (action == GLFW_PRESS && key == GLFW_KEY_O) {
        adding_point ++;
        if (adding_point >= num_points) restart_poly;
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
    points = random_points(50, num_points);
    restart_poly();
}
extern void loop_program(void)
{
    paint_points_c(Canvas3D, points, num_points, "b", 5);
    draw_polyhedron(&poly);
    draw_polyhedron_winding_order(&poly, "k", 5);
    convex_hull(points, num_points);
    // paint_points_c(Canvas3D, &points[adding_point], 1, "k", 20);
    // view_in_poly();
}
extern void close_program(void)
{
}
