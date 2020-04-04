#include "geometry.h"
#include "helper_definitions.h"

Polyhedron new_polyhedron(void)
{
    Polyhedron poly = {0};
    // If this function is not used to create a polyhedron, then these values triggering the calculation of number of features
    // won't be set.
    poly.num_points = -1;
    poly.num_edges = -1;
    poly.num_triangles = -1;
    return poly;
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
    // Nullify incident triangles' reference to this edge.
    for (int i = 0; i < 2; i++) {
        if (e->triangles[i] == NULL) continue;
        for (int j = 0; j < 3; j++) {
            if (e->triangles[i]->edges[j] == e) e->triangles[i]->edges[j] = NULL;
        }
    }
    dl_remove(&poly->edges, e);
}
void polyhedron_remove_triangle(Polyhedron *poly, PolyhedronTriangle *t)
{
    // Nullify incident edges reference to this triangle.
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            if (t->edges[i] != NULL) if (t->edges[i]->triangles[j] == t) t->edges[i]->triangles[j] = NULL;
        }
    }
    dl_remove(&poly->triangles, t);
}
int polyhedron_num_points(Polyhedron *poly)
{
    if (poly->num_points == -1) {
        int n = 0;
        PolyhedronPoint *p = poly->points.first;
        while (p != NULL) { n++; p = p->next; }
        poly->num_points = n;
        return n;
    } else return poly->num_points;
}
int polyhedron_num_edges(Polyhedron *poly)
{
    if (poly->num_edges == -1) {
        int n = 0;
        PolyhedronEdge *e = poly->edges.first;
        while (e != NULL) { n++; e = e->next; }
        poly->num_edges = n;
        return n;
    } else return poly->num_edges;
}
int polyhedron_num_triangles(Polyhedron *poly)
{
    if (poly->num_triangles == -1) {
        int n = 0;
        PolyhedronTriangle *t = poly->triangles.first;
        while (t != NULL) { n++; t = t->next; }
        poly->num_triangles = n;
        return n;
    } else return poly->num_triangles;
}


void print_polyhedron(Polyhedron *p)
{
    PolyhedronPoint *point = p->points.first;
    printf("Points\n");
    printf("--------------------------------------------------------------------------------\n");
    int num_points = 0;
    while (point != NULL) {
        point->print_mark = num_points; // give this point a number.
        num_points ++;
        point = point->next;
    }
    printf("%d points\n", num_points);
    printf("Edges\n");
    printf("--------------------------------------------------------------------------------\n");
    PolyhedronEdge *edge = p->edges.first;
    int num_edges = 0;
    while (edge != NULL) {
        printf("e%d: %d->%d\n", num_edges, edge->a->print_mark, edge->b->print_mark);
        edge->print_mark = num_edges;
        num_edges ++;
        edge = edge->next;
    }
    printf("Triangles\n");
    printf("--------------------------------------------------------------------------------\n");
    PolyhedronTriangle *t = p->triangles.first;
    int num_triangles = 0;
    while (t != NULL) {
        printf("t%d : %d, %d, %d\n", num_triangles, t->points[0]->print_mark, t->points[1]->print_mark, t->points[2]->print_mark);
        t->print_mark = num_triangles;
        num_triangles ++;
        t = t->next;
    }
}

void draw_polyhedron_winding_order(Polyhedron *poly, char *color_str, float line_width, mat4x4 *matrix)
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
            if (matrix != NULL) {
                paint_line_cv(Canvas3D, mat4x4_vec3(matrix, ps[i]), mat4x4_vec3(matrix, ps[i+1]), color_str, line_width);
            } else {
                paint_line_cv(Canvas3D, ps[i], ps[i+1], color_str, line_width);
            }
        }
        vec3 end = vec3_lerp(ps[2], ps[0], 0.8);
        if (matrix != NULL) {
            paint_line_cv(Canvas3D, mat4x4_vec3(matrix, ps[2]), mat4x4_vec3(matrix, end), color_str, line_width);
        } else {
            paint_line_cv(Canvas3D, ps[2], end, color_str, line_width);
        }
        t = t->next;
    }
}

void draw_polyhedron(Polyhedron *p, mat4x4 *matrix)
{
    PolyhedronPoint *point = p->points.first;
    while (point != NULL) {
        if (matrix != NULL) {
            vec3 pp = mat4x4_vec3(matrix, point->position);
            paint_points_c(Canvas3D, &pp, 1, "r", 10);
        } else {
            paint_points_c(Canvas3D, &point->position, 1, "r", 10);
        }
        point = point->next;
    }
    PolyhedronEdge *edge = p->edges.first;
    while (edge != NULL) {
        if (matrix != NULL) {
            paint_line_cv(Canvas3D, mat4x4_vec3(matrix, edge->a->position), mat4x4_vec3(matrix, edge->b->position), "r", 5);
        } else {
            paint_line_cv(Canvas3D, edge->a->position, edge->b->position, "r", 5);
        }
        edge = edge->next;
    }
    // PolyhedronTriangle *t = p->triangles.first;
    // while (t != NULL) {
    //     if (matrix != NULL) {
    //         paint_triangle_v(Canvas3D, mat4x4_vec3(matrix, t->points[0]->position), mat4x4_vec3(matrix, t->points[1]->position), mat4x4_vec3(matrix, t->points[2]->position), new_vec4(0.2,0.2,0.6,0.354));
    //     } else {
    //         paint_triangle_v(Canvas3D, t->points[0]->position, t->points[1]->position, t->points[2]->position, new_vec4(0.2,0.2,0.6,0.354));
    //     }
    //     t = t->next;
    // }
}
void draw_polyhedron2(Polyhedron *p, mat4x4 *matrix, char *color_str, float line_width)
{
    PolyhedronEdge *edge = p->edges.first;
    while (edge != NULL) {
        if (matrix != NULL) {
            paint_line_cv(Canvas3D, mat4x4_vec3(matrix, edge->a->position), mat4x4_vec3(matrix, edge->b->position), color_str, line_width);
        } else {
            paint_line_cv(Canvas3D, edge->a->position, edge->b->position, color_str, line_width);
        }
        edge = edge->next;
    }
/*
    PolyhedronPoint *point = p->points.first;
    while (point != NULL) {
        if (matrix != NULL) {
            vec3 pp = mat4x4_vec3(matrix, point->position);
            paint_points_c(Canvas3D, &pp, 1, color_str, line_width);
        } else {
            paint_points_c(Canvas3D, &point->position, 1, color_str, line_width);
        }
        point = point->next;
    }
*/
}

/*================================================================================
    3-dimensional convex hull. Returns the hull as a polyhedron.
================================================================================*/
// Definitions of marks this algorithm makes on features, during processing.
#define VISIBLE true
#define INVISIBLE false
#define NEEDED 0x1
#define BOUNDARY 0x2
Polyhedron convex_hull(vec3 *points, int num_points)
{
    Polyhedron poly = new_polyhedron();
    if (num_points < 4) {
        // The points are their own convex hull.
        //-- currently just returning the points in this case.
        for (int i = 0; i < num_points; i++) polyhedron_add_point(&poly, points[i]);
        return poly;
    }
    // Start up a polyhedron data structure as a tetrahedron.
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
    for (int i = 4; i < num_points; i++) {
        // Clean up the marks for points and edges (not neccessary for triangles, since they are always set).
        PolyhedronPoint *p = poly.points.first;
        while (p != NULL) {
            p->mark = 0;
            p->saved_edge = NULL;
            p = p->next;
        }
        PolyhedronEdge *e = poly.edges.first;
        while (e != NULL) { 
            e->mark = 0;
            e = e->next;
        }
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
            continue; // The point is in the hull so far.
        }
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
        t = poly.triangles.first;
        while (t != NULL) {
            if (t->mark == VISIBLE) {
                PolyhedronTriangle *to_remove = t;
                t = t->next;
                polyhedron_remove_triangle(&poly, to_remove);
            } else t = t->next;
        }
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
        // Add the new point, and new edges and triangles to create a cone from the new point to the border.
        PolyhedronPoint *new_point = polyhedron_add_point(&poly, points[i]);
        e = poly.edges.first;
        while (e != NULL) {
            if ((e->mark & BOUNDARY) != 0) {
                // Determine whether a->b is in line with the winding of the invisible triangle incident to ab, and adjust accordingly.
                bool reverse = false;
                for (int i = 0; i < 2; i++) {
                    if (e->triangles[i] != NULL) {
                        for (int j = 0; j < 3; j++) {
                            if (e->a == e->triangles[i]->points[j] && e->b == e->triangles[i]->points[(j+1)%3]) reverse = true;
                        }
                        break;
                    }
                }
                // In winding order, introduce two new edges, unless an edge has already been made.
                PolyhedronPoint *p1 = reverse ? e->b : e->a;
                PolyhedronPoint *p2 = reverse ? e->a : e->b;
                PolyhedronEdge *e1 = p1->saved_edge;
                if (e1 == NULL) {
                    e1 = polyhedron_add_edge(&poly, new_point, p1);
                    p1->saved_edge = e1;
                }
                PolyhedronEdge *e2 = p2->saved_edge;
                if (e2 == NULL) {
                    e2 = polyhedron_add_edge(&poly, new_point, p2);
                    p2->saved_edge = e2;
                }
                // Use these to form a new triangle.
                polyhedron_add_triangle(&poly, new_point, p1, p2, e1, e, e2);
            }
            e = e->next;
        }
    }
    return poly;
}


bool point_in_convex_polyhedron(vec3 p, Polyhedron poly)
{
    // Uses "visibility" from the point. If any triangle is visible (as in, from the point of view of the point, the triangle is non-degenerate and in anti-clockwise order),
    // then the point is outside the convex polyhedron.
    bool any_visible = false;
    PolyhedronTriangle *t = poly.triangles.first;
    while (t != NULL) {
        float v = tetrahedron_6_times_volume(t->points[0]->position, t->points[1]->position, t->points[2]->position, p);
        if (v < 0) {
            any_visible = true;
        }
        t = t->next;
    }
    return !any_visible;
}

float polyhedron_volume(Polyhedron poly)
{
    float volume = 0.0;
    PolyhedronTriangle *t = poly.triangles.first;
    vec3 zero = vec3_zero();
    while (t != NULL) {
        volume += tetrahedron_6_times_volume(t->points[0]->position, t->points[1]->position, t->points[2]->position, zero);
        t = t->next;
    }
    return volume / 6.0;
}

// This is for a general polyhedron, so no hill-climbing is done.
vec3 polyhedron_extreme_point(Polyhedron poly, vec3 direction)
{
    PolyhedronPoint *p = poly.points.first;
    float d = vec3_dot(p->position, direction);
    vec3 v = p->position;
    while ((p = p->next) != NULL) {
        float new_d = vec3_dot(p->position, direction);
        if (new_d > d) {
            d = new_d;
            v = p->position;
        }
    }
    return v;
}

// Extract the points from the polyhedron as an array on the heap.
vec3 *polyhedron_points(Polyhedron poly)
{
    int n = polyhedron_num_points(&poly);
    vec3 *points = malloc(sizeof(vec3) * n);
    mem_check(points);
    PolyhedronPoint *p = poly.points.first;
    int i = 0;
    while (p != NULL) {
        points[i++] = p->position;
        p = p->next;
    }
    return points;
}

vec3 polyhedron_center_of_mass(Polyhedron poly)
{
    // Calculate the center of mass (assuming the mass is uniform).
    PolyhedronTriangle *tri = poly.triangles.first;
    vec3 center_of_mass = new_vec3(0,0,0);
    // The center of mass is found by taking the weighted sum of the centroids of tetrahedra connecting the origin to each triangle,
    // weighted by the signed volumes of each tetrahedron.
    // ---I think this method works, at least for convex polyhedra, since if the origin is in the center,
    // ---then this works, and it would not become incorrect when moving the origin past the boundary.
    float volume_times_6 = 0;
    while (tri != NULL) {
        // Calculate the centroid of the tetrahedron containing the origin and the points of the triangle.
        vec3 centroid = vec3_mul(vec3_add(vec3_add(tri->points[0]->position, tri->points[1]->position), tri->points[2]->position), 0.25);
        // Calculate the volume of this same tetrahedron.
        float v = tetrahedron_6_times_volume(new_vec3(0,0,0), tri->points[0]->position, tri->points[1]->position, tri->points[2]->position);
        volume_times_6 += v;
        center_of_mass = vec3_add(center_of_mass, vec3_mul(centroid, v));
        tri = tri->next;
    }
    center_of_mass = vec3_mul(center_of_mass, 1.0 / volume_times_6);
    return center_of_mass;
}


/*--------------------------------------------------------------------------------
    Polytope methods. Polytopes are represented by only their points, and their polyhedron can be recovered at any time by taking the convex hull.
    (Polyhedron representation may not be consistent due to triangulation of faces).
--------------------------------------------------------------------------------*/
vec3 polytope_center_of_mass(vec3 *points, int num_points)
{
    Polyhedron hull = convex_hull(points, num_points);
    vec3 center_of_mass = polyhedron_center_of_mass(hull);
    //----------////// Destroy the polyhedron.
    return center_of_mass;
}
