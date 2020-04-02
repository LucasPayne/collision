/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

/*================================================================================
----------------------------------------------------------------------------------
        Convex polyhedra collision.
----------------------------------------------------------------------------------
    Collision and contact information is computed for two convex polyhedra.
    The polyhedra are given as point clouds, which define their convex hulls.
    The GJK (Gilbert-Johnson-Keerthi) algorithm is used to descend a simplex
    through the Minkowski difference, or configuration space obstacle, of the polyhedra.
    If the simplex ever bounds the origin, the polyhedra are intersecting (as for some
    points a in A and b in B, a - b = 0 => a = b).

    If they are not intersecting, the closest points of the two polyhedra can be computed from
    the closest point on the CSO to the origin.
    If they are colliding, contact information is wanted. The minimum separating vector is given,
    computed by the expanding polytope algorithm (EPA). This algorithm progressively expands
    a sub-polytope of the CSO in order to find the closest point on the CSO boundary to the origin,
    whose negative is the separating vector, the minimal translation to move the CSO so that it does not
    bound the origin. This can be used to infer the contact normal and contact points on each polyhedron.
================================================================================*/
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
    *A_support = support_index(A, A_len, direction);
    *B_support = support_index(B, B_len, vec3_neg(direction));
    return vec3_sub(A[*A_support], B[*B_support]);
}
bool convex_hull_intersection(vec3 *A, int A_len, vec3 *B, int B_len)
{
    // Initialize the simplex as a line segment.
    vec3 simplex[4];
    int indices_A[4];
    int indices_B[4];
    int n = 2;
    simplex[0] = cso_support(A, A_len, B, B_len, new_vec3(1,1,1), &indices_A[0], &indices_B[0]);
    simplex[1] = cso_support(A, A_len, B, B_len, vec3_neg(simplex[0]), &indices_A[1], &indices_B[1]);
    vec3 origin = vec3_zero();

    // Go into a loop, computing the closest point on the simplex and expanding it in the opposite direction (from the origin),
    // and removing simplex points to maintain n <= 4.
    while (1) {
        vec3 c = closest_point_on_simplex(n, simplex, origin);
        vec3 dir = vec3_neg(c);

        // If the simplex is a tetrahedron and contains the origin, the CSO contains the origin.
        if (n == 4 && point_in_tetrahedron(simplex[0],simplex[1],simplex[2],simplex[3], origin)) {
            paint_points_c(Canvas3D, &origin, 1, "tg", 50);
            //---EPA here.
            return true;
        }
        int A_index, B_index;
        vec3 new_point = cso_support(A, A_len, B, B_len, dir, &A_index, &B_index);

        bool on_simplex = false;
        for (int i = 0; i < n; i++) {
            if (A_index == indices_A[i] && B_index == indices_B[i]) {
                on_simplex = true;
                break;
            }
        }
        if (n == 4 && !on_simplex) {
            int replace = simplex_extreme_index(n, simplex, c);
            simplex[replace] = new_point;
            indices_A[replace] = A_index;
            indices_B[replace] = B_index;
            ///////////////////////////////////////////////////////////////////////////////////////////////////////
            //----This check seems to fix an infinite loop bug here, but I am not sure if the reasoning is correct.
            ///////////////////////////////////////////////////////////////////////////////////////////////////////
            if (vec3_dot(new_point, dir) <= 0) {
                paint_points_c(Canvas3D, &origin, 1, "tr", 50);
                vec3 closest_on_poly = closest_point_on_tetrahedron_to_point(simplex[0], simplex[1], simplex[2], simplex[3], origin);
                paint_points_c(Canvas3D, &closest_on_poly, 1, "tb", 50);
                return false;
            }
        } else if (n == 3 && on_simplex) {
            paint_points_c(Canvas3D, &origin, 1, "tr", 50);
            vec3 closest_on_poly = closest_point_on_triangle_to_point(simplex[0], simplex[1], simplex[2], origin);
            paint_points_c(Canvas3D, &closest_on_poly, 1, "tb", 50);
            return false;
        } else if (n == 2 && on_simplex) {
            paint_points_c(Canvas3D, &origin, 1, "tr", 50);
            vec3 closest_on_poly = closest_point_on_line_segment_to_point(simplex[0], simplex[1], origin);
            paint_points_c(Canvas3D, &closest_on_poly, 1, "tb", 50);
            return false;
        } else if (n == 1 && on_simplex) {
            paint_points_c(Canvas3D, &origin, 1, "tr", 50);
            paint_points_c(Canvas3D, &simplex[0], 1, "tb", 50);
            return false;
        } else if (!on_simplex) {
            simplex[n] = new_point;
            indices_A[n] = A_index;
            indices_B[n] = B_index;
            n++;
        } else {
            int remove = simplex_extreme_index(n, simplex, c);
            for (int j = remove; j < n - 1; j++) {
                simplex[j] = simplex[j + 1];
                indices_A[j] = indices_A[j + 1];
                indices_B[j] = indices_B[j + 1];
            }
            n--;
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
        vec3 c = closest_point_on_simplex(n, simplex, origin);
        vec3 dir = vec3_neg(c);
        if (n == 4 && point_in_tetrahedron(simplex[0],simplex[1],simplex[2],simplex[3], origin)) {
            //show_simplex(n, simplex, "g");
            paint_points_c(Canvas3D, &origin, 1, "tg", 50);

            // Find the closest point on the boundary.
            // The negative of this will be the minimal separating vector the polyhedron has from the origin.
            // -------------------------------------------------------------------------------
            // Brute force it for comparison.
            PolyhedronTriangle *tri = poly.triangles.first;
            vec3 brute_p = closest_point_on_triangle_to_point(tri->points[0]->position,tri->points[1]->position,tri->points[2]->position, origin);
            while ((tri = tri->next) != NULL) {
                vec3 new_p = closest_point_on_triangle_to_point(tri->points[0]->position,tri->points[1]->position,tri->points[2]->position, origin);
                if (vec3_dot(new_p, new_p) < vec3_dot(brute_p, brute_p)) brute_p = new_p;
            }
	    paint_points_c(Canvas3D, &brute_p, 1, "g", 30);

            // Perform the expanding polytope algorithm.

            // Instead of using a fancy data-structure, the polytope is maintained by keeping
            // a pool. Entries can be nullified, and entries readded in those empty spaces, but linear iterations still need to
            // check up to *_len features. If the arrays are not long enough, then this fails.
            float points[1024 * 3] = {-1}; //----should be indices. If coord is -1, this is definitely buggy.
            const int points_n = 3;
            int points_len = 0;
            int16_t triangles[1024 * 6] = {-1};
            const int triangles_n = 6;
            int triangles_len = 0;
            int16_t edges[1024 * 2] = {-1};
            const int edges_n = 2;
            int edges_len = 0;

            // For efficiency, features are added by this simple macro'd routine. e.g.,
            //     int new_tri_index;
            //     new_feature_index(triangles, new_tri_index);
            // will get the index of an available triangle slot.
            static const char *EPA_error_string = ERROR_ALERT "Expanding polytope algorithm: Fixed-length feature lists failed to be sufficient.\n";
            #define new_feature_index(TYPE,INDEX)\
            {\
                bool found = false;\
                for (int i = 0; i < TYPE ## _len; i++) {\
                    if (TYPE [TYPE ## _n * i] == -1) {\
                        ( INDEX ) = i;\
                        found = true;\
                        break;\
                    }\
                }\
                if (!found) {\
                    if (TYPE ## _len == 1024) {\
                        fprintf(stderr, EPA_error_string);\
                        exit(EXIT_FAILURE);\
                    }\
                    ( INDEX ) = TYPE ## _len ++;\
                }\
            }
            #define add_point(VEC, INDEX) {\
                new_feature_index(points, ( INDEX ));\
                points[points_n * ( INDEX )] = ( VEC ).vals[0];\
                points[points_n * ( INDEX ) + 1] = ( VEC ).vals[1];\
                points[points_n * ( INDEX ) + 2] = ( VEC ).vals[2];\
            }
            // Edge: references its end points.
            #define add_edge(AI,BI,INDEX) {\
                new_feature_index(edges, ( INDEX ));\
                edges[edges_n * ( INDEX )] = ( AI );\
                edges[edges_n * ( INDEX ) + 1] = ( BI );\
            }
            // Triangle: References its points in anti-clockwise winding order, abc, and references its edges, ab, bc, ca.
            #define add_triangle(AI,BI,CI) {\
                int index;\
                new_feature_index(triangles, index);\
                triangles[triangles_n * index] = ( AI );\
                triangles[triangles_n * index + 1] = ( BI );\
                triangles[triangles_n * index + 2] = ( CI );\
                add_edge(( AI ),( BI ),triangles[triangles_n * index + 3]);\
                add_edge(( BI ),( CI ),triangles[triangles_n * index + 4]);\
                add_edge(( CI ),( AI ),triangles[triangles_n * index + 5]);\
            }

            #define show_polytope() {\
                for (int i = 0; i < edges_len; i++) {\
                    if (edges[edges_n*i] == -1) continue;\
                    paint_line_cv(Canvas3D, *((vec3 *) &points[points_n*edges[edges_n*i]]), *((vec3 *) &points[points_n*edges[edges_n*i+1]]), "y", 40);\
                }\
            }
/*
                for (int i = 0; i < triangles_len; i++) {\
                    if (triangles[triangles_n*i] == -1) continue;\
                    paint_triangle_cv(Canvas3D, *((vec3 *) &points[points_n*triangles[triangles_n*i]]),*((vec3 *) &points[points_n*triangles[triangles_n*i+1]]),*((vec3 *) &points[points_n*triangles[triangles_n*i+2]]), "tk");\
                }\
*/

            float v = tetrahedron_6_times_volume(simplex[0],simplex[1],simplex[2],simplex[3]);
            if (v < 0) {
                // If the tetrahedron has negative volume, swap two entries, forcing the winding order to be anti-clockwise from outside.
                vec3 temp = simplex[0];
                simplex[0] = simplex[1];
                simplex[1] = temp;
            }
            //---since it is known that everything is empty, it would be more efficient to just hardcode the initial tetrahedron.
            int dummy; // since the macro saves the index.
            for (int i = 0; i < 4; i++) {
                add_point(simplex[i], dummy);
            }
            add_triangle(0,1,2);
            add_triangle(1,0,3);
            add_triangle(2,1,3);
            add_triangle(0,2,3);

            // The initial tetrahedron has been set up. Proceed with EPA.
            int counter = 0;
            while (1) {
                counter ++;

                check() {
                    show_polytope();
                    return;
                }

                // Find the closest triangle to the origin.
                float min_d = -1;
                int closest_triangle_index = -1;
                for (int i = 0; i < triangles_len; i++) {
                    if (triangles[triangles_n*i] == -1) continue;
                    vec3 a = *((vec3 *) &points[points_n*triangles[triangles_n*i]]);
                    vec3 b = *((vec3 *) &points[points_n*triangles[triangles_n*i+1]]);
                    vec3 c = *((vec3 *) &points[points_n*triangles[triangles_n*i+2]]);
                    vec3 p = point_to_triangle_plane(a,b,c, origin);
                    float new_d = vec3_dot(p, p);
                    if (min_d == -1 || new_d < min_d) {
                        min_d = new_d;
                        closest_triangle_index = i;
                    }
                }
                vec3 a = *((vec3 *) &points[points_n*triangles[triangles_n*closest_triangle_index]]);
                vec3 b = *((vec3 *) &points[points_n*triangles[triangles_n*closest_triangle_index + 1]]);
                vec3 c = *((vec3 *) &points[points_n*triangles[triangles_n*closest_triangle_index + 2]]);
                check() {
                    show_polytope();
                    vec3 p = point_to_triangle_plane(a,b,c, origin);
                    paint_points_c(Canvas3D, &p, 1, "k", 25);
                    paint_line_cv(Canvas3D, origin, p, "k", 20);
                    return;
                }

                // Find an extreme point in the direction from the origin to the closest point on the polytope boundary.
                // The convex hull of the points of the polytope adjoined with this new point will be computed.
                vec3 expand_to = vec3_cross(vec3_sub(b, a), vec3_sub(c, a));
                vec3 new_point = polyhedron_extreme_point(poly, expand_to);
                bool new_point_on_polytope = false;
                for (int i = 0; i < points_len; i++) {
                    // Unreferenced but non-nullified points can't be on the convex hull, so they can be checked against without problems.
                    if (points[points_n*i] == -1) continue;
                    if (memcmp(&points[points_n*i], &new_point, sizeof(float)*3) == 0) {
                        new_point_on_polytope = true;
                        break;
                    }
                }
                if (new_point_on_polytope) {
                    // The closest triangle is on the border of the polyhedron, so the closest point on this triangle is the closest point
                    // to the border of the polyhedron.
                    vec3 closest_point = point_to_triangle_plane(a,b,c, origin);
                    paint_points_c(Canvas3D, &closest_point, 1, "tp", 300);
                    return;
                }

                // Remove the visible triangles and their directed edges. Points do not need to be nullified.
                for (int i = 0; i < triangles_len; i++) {
                    if (triangles[triangles_n*i] == -1) continue;
                    vec3 a = *((vec3 *) &points[points_n*triangles[triangles_n*i]]);
                    vec3 b = *((vec3 *) &points[points_n*triangles[triangles_n*i+1]]);
                    vec3 c = *((vec3 *) &points[points_n*triangles[triangles_n*i+2]]);
                    vec3 n = vec3_cross(vec3_sub(b, a), vec3_sub(c, a));
                    float v = tetrahedron_6_times_volume(a,b,c, new_point);
                    if (v < 0) {
                        // Remove this triangle, as it is visible from the new point.
                        edges[edges_n*triangles[triangles_n*i+3]] = -1;
                        edges[edges_n*triangles[triangles_n*i+4]] = -1;
                        edges[edges_n*triangles[triangles_n*i+5]] = -1;
                        triangles[triangles_n*i] = -1;
                    }
                }
                int new_point_index;
                add_point(new_point, new_point_index);

                // Search for boundary edges and save them in an array.
                int boundary_len = 0;
                int16_t boundary_edges[1024];
                for (int i = 0; i < edges_len; i++) {
                    if (edges[edges_n*i] == -1) continue;
                    // Search for an incident triangle, by looking for the opposite edge (with reversed direction).
                    bool boundary = true;
                    for (int j = 0; j < edges_len; j++) {
                        if (edges[edges_n*j] == -1) continue;
                        if (   memcmp(&points[points_n*edges[edges_n*i]], &points[points_n*edges[edges_n*j+1]], sizeof(float)*3) == 0
                            && memcmp(&points[points_n*edges[edges_n*i+1]], &points[points_n*edges[edges_n*j]], sizeof(float)*3) == 0) {
                            // This is the opposite edge, so a boundary edge has not been found.
                            boundary = false;
                            break;
                        }
                    }
                    if (boundary) {
                        // Instead of adding the triangle straight away, save the boundary edge of the new triangle in the array.
                        // This is because the edges are still being iterated, and if new ones are added, they could be registered as boundary edges.
                        boundary_edges[2*boundary_len] = edges[edges_n*i+1];
                        boundary_edges[2*boundary_len+1] = edges[edges_n*i];
                        boundary_len ++; //--- not checking out-of-space.
                    }
                }
                // Add the triangles of the extending cone.
                for (int i = 0; i < boundary_len; i++) {
                    add_triangle(boundary_edges[2*i], boundary_edges[2*i+1], new_point_index);
                }
            }
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
            //show_simplex(n, simplex, "r");
            paint_points_c(Canvas3D, &origin, 1, "tr", 50);
            vec3 closest_on_poly = closest_point_on_triangle_to_point(simplex[0], simplex[1], simplex[2], origin);
            paint_points_c(Canvas3D, &closest_on_poly, 1, "tb", 50);
            return;
        } else if (n == 2 && on_simplex) {
            //show_simplex(n, simplex, "r");
            paint_points_c(Canvas3D, &origin, 1, "tr", 50);
            vec3 closest_on_poly = closest_point_on_line_segment_to_point(simplex[0], simplex[1], origin);
            paint_points_c(Canvas3D, &closest_on_poly, 1, "tb", 50);
        } else if (n == 1 && on_simplex) {
            paint_points_c(Canvas3D, &origin, 1, "tr", 50);
            paint_points_c(Canvas3D, &simplex[0], 1, "tb", 50);
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
            //show_simplex(n, simplex, "k");
            return;
        }
    }
}


Polyhedron polyA;
Polyhedron polyB;
vec3 *A = NULL;
int A_len;
vec3 *B = NULL;
int B_len;
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
void test_gjk(void)
{
    if (A != NULL) free(A);
    if (B != NULL) free(B);
    int A_len = polyhedron_num_points(&polyA);
    int B_len = polyhedron_num_points(&polyB);
    A = malloc(sizeof(float)*3 * A_len);
    mem_check(A);
    B = malloc(sizeof(float)*3 * B_len);
    mem_check(B);
    PolyhedronPoint *p = polyA.points.first;
    int i = 0;
    while (p != NULL) {
        A[i] = p->position;
        p = p->next;
        i++;
    }
    p = polyB.points.first;
    i = 0;
    while (p != NULL) {
        B[i] = p->position;
        p = p->next;
        i++;
    }

    convex_hull_intersection(A, A_len, B, B_len);
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
    float y_move = 0;
    if (arrow_key_down(Up)) {
        y_move += 60;
    }
    if (arrow_key_down(Down)) {
        y_move -= 60;
    }
    if (y_move != 0) {
        PolyhedronPoint *p = polyA.points.first;
        while (p != NULL) {
            p->position.vals[1] += y_move * dt;
            p = p->next;
        }
        compute_minkowski_difference(polyA, polyB);
    }

    draw_polyhedron2(&polyA, NULL, "tb", 5);
    draw_polyhedron2(&polyB, NULL, "tr", 5);
    draw_polyhedron2(&mink, NULL, "tk", 3);
    vec3 origin = vec3_zero();
    paint_points_c(Canvas3D, &origin, 1, "p", 25);

    test_gjk();
}
extern void close_program(void)
{
}
