/*================================================================================
================================================================================*/
#include "Engine.h"

//================================================================================
// for testing and debugging.
Polyhedron compute_minkowski_difference(Polyhedron A, Polyhedron B)
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
    Polyhedron diff = convex_hull(points, num_points);
    free(points);
    return diff;
}
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
static int support_index(vec3 *points, int num_points, mat4x4 *matrix, vec3 direction)
{
    //---Basic rearrangement can allow the avoidance of most matrix-vector multiplies here.
    float d = vec3_dot(mat4x4_vec3(matrix, points[0]), direction); // At least one point must be given.
    int index = 0;
    for (int i = 1; i < num_points; i++) {
        float new_d = vec3_dot(mat4x4_vec3(matrix, points[i]), direction);
        if (new_d > d) {
            d = new_d;
            index = i;
        }
    }
    return index;
}
bool convex_hull_intersection(vec3 *A, int A_len, mat4x4 *A_matrix, vec3 *B, int B_len, mat4x4 *B_matrix, GJKManifold *manifold)
{
#define DEBUG 1 // Turn this flag on to visualize some things.
    // Initialize the simplex as a line segment.
    vec3 simplex[4];
    int indices_A[4];
    int indices_B[4];
    int n = 2;
    // cso: Configuration space obstacle, another name for the Minkowski difference of two sets.
    // This macro gives the support vector in the Minkowski difference, and also gives the indices of the points in A and B whose difference is that support vector.
    #define cso_support(DIRECTION,SUPPORT,INDEX_A,INDEX_B)\
    {\
        ( INDEX_A ) = support_index(A, A_len, A_matrix, ( DIRECTION ));\
        ( INDEX_B ) = support_index(B, B_len, B_matrix, vec3_neg(( DIRECTION )));\
        ( SUPPORT ) = vec3_sub(mat4x4_vec3(A_matrix, A[( INDEX_A )]), mat4x4_vec3(B_matrix, B[( INDEX_B )]));\
    }
    cso_support(new_vec3(1,1,1), simplex[0], indices_A[0], indices_B[0]);
    cso_support(vec3_neg(simplex[0]), simplex[1], indices_A[1], indices_B[1]);
    vec3 origin = vec3_zero();

    // Go into a loop, computing the closest point on the simplex and expanding it in the opposite direction (from the origin),
    // and removing simplex points to maintain n <= 4.
    while (1) {
        vec3 c = closest_point_on_simplex(n, simplex, origin);
        vec3 dir = vec3_neg(c);

        // If the simplex is a tetrahedron and contains the origin, the CSO contains the origin.
        if (n == 4 && point_in_tetrahedron(simplex[0],simplex[1],simplex[2],simplex[3], origin)) {
            /*
            if (DEBUG) {
                paint_points_c(Canvas3D, &origin, 1, "tg", 50);
                // Brute force for comparison and debugging.
                Polyhedron poly = compute_minkowski_difference(convex_hull(A, A_len), convex_hull(B, B_len));
                PolyhedronTriangle *tri = poly.triangles.first;
                vec3 brute_p = closest_point_on_triangle_to_point(tri->points[0]->position,tri->points[1]->position,tri->points[2]->position, origin);
                while ((tri = tri->next) != NULL) {
                    vec3 new_p = closest_point_on_triangle_to_point(tri->points[0]->position,tri->points[1]->position,tri->points[2]->position, origin);
                    if (vec3_dot(new_p, new_p) < vec3_dot(brute_p, brute_p)) brute_p = new_p;
                }
	        paint_points_c(Canvas3D, &brute_p, 1, "g", 30);
            }
            */
            // Perform the expanding polytope algorithm.
            // Instead of using a fancy data-structure, the polytope is maintained by keeping
            // a pool. Entries can be nullified, and entries re-added in those empty spaces, but linear iterations still need to
            // check up to *_len features. If the arrays are not long enough, then this fails.
            int16_t points[1024 * 2] = {-1};
            const int points_n = 2;
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
            // Point: References the indices of the points of A and B for which the difference is this point.
            #define add_point(POINT_INDEX_A,POINT_INDEX_B,INDEX) {\
                new_feature_index(points, ( INDEX ));\
                points[points_n * ( INDEX )] = ( POINT_INDEX_A );\
                points[points_n * ( INDEX ) + 1] = ( POINT_INDEX_B );\
            }
            // Edge: References its end points.
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
            float v = tetrahedron_6_times_volume(simplex[0],simplex[1],simplex[2],simplex[3]);
            if (v < 0) {
                // If the tetrahedron has negative volume, swap two entries, forcing the winding order to be anti-clockwise from outside.
                vec3 temp = simplex[0];
                int tempA = indices_A[0];
                int tempB = indices_B[0];
                simplex[0] = simplex[1];
                indices_A[0] = indices_A[1];
                indices_B[0] = indices_B[1];
                simplex[1] = temp;
                indices_A[1] = tempA;
                indices_B[1] = tempB;
            }
            //---since it is known that everything is empty, it would be more efficient to just hardcode the initial tetrahedron.
            int dummy; // since the macro saves the index.
            for (int i = 0; i < 4; i++) {
                add_point(indices_A[i], indices_B[i], dummy);
            }
            add_triangle(0,1,2);
            add_triangle(1,0,3);
            add_triangle(2,1,3);
            add_triangle(0,2,3);

            // The initial tetrahedron has been set up. Proceed with EPA.
	    int COUNTER = 0; // for debugging.
            while (1) {
                COUNTER ++;
                // Find the closest triangle to the origin.
                float min_d = -1;
                int closest_triangle_index = -1;
                vec3 closest_point;
                for (int i = 0; i < triangles_len; i++) {
                    if (triangles[triangles_n*i] == -1) continue;

                    vec3 a = vec3_sub(mat4x4_vec3(A_matrix, A[points[points_n*triangles[triangles_n*i+0]]]), mat4x4_vec3(B_matrix, B[points[points_n*triangles[triangles_n*i+0] + 1]]));
                    vec3 b = vec3_sub(mat4x4_vec3(A_matrix, A[points[points_n*triangles[triangles_n*i+1]]]), mat4x4_vec3(B_matrix, B[points[points_n*triangles[triangles_n*i+1] + 1]]));
                    vec3 c = vec3_sub(mat4x4_vec3(A_matrix, A[points[points_n*triangles[triangles_n*i+2]]]), mat4x4_vec3(B_matrix, B[points[points_n*triangles[triangles_n*i+2] + 1]]));
                    vec3 p = point_to_triangle_plane(a,b,c, origin);
                    float new_d = vec3_dot(p, p);
                    if (min_d == -1 || new_d < min_d) {
                        min_d = new_d;
                        closest_triangle_index = i;
                        closest_point = p;
                    }
                }
                vec3 a = vec3_sub(mat4x4_vec3(A_matrix, A[points[points_n*triangles[triangles_n*closest_triangle_index+0]]]), mat4x4_vec3(B_matrix, B[points[points_n*triangles[triangles_n*closest_triangle_index+0] + 1]]));
                vec3 b = vec3_sub(mat4x4_vec3(A_matrix, A[points[points_n*triangles[triangles_n*closest_triangle_index+1]]]), mat4x4_vec3(B_matrix, B[points[points_n*triangles[triangles_n*closest_triangle_index+1] + 1]]));
                vec3 c = vec3_sub(mat4x4_vec3(A_matrix, A[points[points_n*triangles[triangles_n*closest_triangle_index+2]]]), mat4x4_vec3(B_matrix, B[points[points_n*triangles[triangles_n*closest_triangle_index+2] + 1]]));

                // Find an extreme point in the direction from the origin to the closest point on the polytope boundary.
                // The convex hull of the points of the polytope adjoined with this new point will be computed.
                // vec3 expand_to = vec3_cross(vec3_sub(b, a), vec3_sub(c, a));
                vec3 expand_to = closest_point;
                int new_point_A_index, new_point_B_index;
                vec3 new_point;
                cso_support(expand_to, new_point, new_point_A_index, new_point_B_index);
                bool new_point_on_polytope = false;
                for (int i = 0; i < points_len; i++) {
                    // Unreferenced but non-nullified points can't be on the convex hull, so they can be checked against without problems.
                    if (points[points_n*i] == -1) continue;
                    if (points[points_n*i] == new_point_A_index && points[points_n*i+1] == new_point_B_index) {
                        new_point_on_polytope = true;
                        break;
                    }
                }
                if (new_point_on_polytope) {
                    // The closest triangle is on the border of the polyhedron, so the closest point on this triangle is the closest point
                    // to the border of the polyhedron.

                    // The separating vector is the minimal translation B must make to separate from A.
                    manifold->separating_vector = closest_point;

                    // Compute the barycentric coordinates of the closest point in terms of the triangle on the boundary of the CSO.
                    // This triangle is the Minkowski difference between a triangle on A and a triangle on B. Use the same barycentric weights
                    // to calculate the corresponding points on the boundaries of A and B.
                    //---need a better way to get these points.
                    vec3 Aa = mat4x4_vec3(A_matrix, A[points[points_n*triangles[triangles_n*closest_triangle_index+0]]]);
                    vec3 Ab = mat4x4_vec3(A_matrix, A[points[points_n*triangles[triangles_n*closest_triangle_index+1]]]);
                    vec3 Ac = mat4x4_vec3(A_matrix, A[points[points_n*triangles[triangles_n*closest_triangle_index+2]]]);
                    vec3 Ba = mat4x4_vec3(B_matrix, B[points[points_n*triangles[triangles_n*closest_triangle_index+0]+1]]);
                    vec3 Bb = mat4x4_vec3(B_matrix, B[points[points_n*triangles[triangles_n*closest_triangle_index+1]+1]]);
                    vec3 Bc = mat4x4_vec3(B_matrix, B[points[points_n*triangles[triangles_n*closest_triangle_index+2]+1]]);
                    vec3 a = vec3_sub(Aa, Ba);
                    vec3 b = vec3_sub(Ab, Bb);
                    vec3 c = vec3_sub(Ac, Bc);
                    vec3 barycentric_coords = point_to_triangle_plane_barycentric(a,b,c,  origin);
                    manifold->A_closest = barycentric_triangle_v(Aa,Ab,Ac,  barycentric_coords);
                    manifold->B_closest = barycentric_triangle_v(Ba,Bb,Bc,  barycentric_coords);
                    // printf("a: ");print_vec3(a);
                    // printf("b: ");print_vec3(b);
                    // printf("c: ");print_vec3(c);
                    // print_vec3(barycentric_coords);
                    // print_vec3(manifold->A_closest);
                    // print_vec3(manifold->B_closest);
                    if (DEBUG) {
                        paint_points_c(Canvas3D, &new_point, 1, "tr", 65);
                        paint_line_cv(Canvas3D, origin, closest_point, "tr", 5);
                        paint_points_c(Canvas3D, &closest_point, 1, "tp", 300);
                        paint_points_c(Canvas3D, &manifold->A_closest, 1, "tr", 30);
                        paint_points_c(Canvas3D, &manifold->B_closest, 1, "tb", 30);
                        paint_line_cv(Canvas3D, manifold->A_closest, manifold->B_closest, "g", 20);
                        // Show the polytope.
                        for (int i = 0; i < edges_len; i++) {
                            if (edges[edges_n*i] == -1) continue;
                            //---untransformed
                            vec3 p1 = vec3_sub(A[points[points_n*edges[edges_n*i+0]]], B[points[points_n*edges[edges_n*i+0] + 1]]);
                            vec3 p2 = vec3_sub(A[points[points_n*edges[edges_n*i+1]]], B[points[points_n*edges[edges_n*i+1] + 1]]);
                            paint_line_cv(Canvas3D, p1, p2, "y", 40);
                        }
                        for (int i = 0; i < triangles_len; i++) {
                            if (triangles[triangles_n*i] == -1) continue;
                            //---untransformed
                            vec3 p1 = vec3_sub(A[points[points_n*triangles[triangles_n*i+0]]], B[points[points_n*triangles[triangles_n*i+0] + 1]]);
                            vec3 p2 = vec3_sub(A[points[points_n*triangles[triangles_n*i+1]]], B[points[points_n*triangles[triangles_n*i+1] + 1]]);
                            vec3 p3 = vec3_sub(A[points[points_n*triangles[triangles_n*i+2]]], B[points[points_n*triangles[triangles_n*i+2] + 1]]);
                            //draw_triangle_winding_order(p1, p2, p3, "p", 10);
                            // paint_triangle_cv(Canvas3D, p1, p2, p3, "tk");
                        }
                    }
                    return true;
                }

                // Remove the visible triangles and their directed edges. Points do not need to be nullified.
                for (int i = 0; i < triangles_len; i++) {
                    if (triangles[triangles_n*i] == -1) continue;
                    vec3 ap = vec3_sub(mat4x4_vec3(A_matrix, A[points[points_n*triangles[triangles_n*i+0]]]), mat4x4_vec3(B_matrix, B[points[points_n*triangles[triangles_n*i+0] + 1]]));
                    vec3 bp = vec3_sub(mat4x4_vec3(A_matrix, A[points[points_n*triangles[triangles_n*i+1]]]), mat4x4_vec3(B_matrix, B[points[points_n*triangles[triangles_n*i+1] + 1]]));
                    vec3 cp = vec3_sub(mat4x4_vec3(A_matrix, A[points[points_n*triangles[triangles_n*i+2]]]), mat4x4_vec3(B_matrix, B[points[points_n*triangles[triangles_n*i+2] + 1]]));

                    vec3 n = vec3_cross(vec3_sub(bp, ap), vec3_sub(cp, ap));
                    float v = tetrahedron_6_times_volume(ap,bp,cp, new_point);
                    if (v < 0) {
                        // Remove this triangle, as it is visible from the new point.
                        edges[edges_n*triangles[triangles_n*i+3]] = -1;
                        edges[edges_n*triangles[triangles_n*i+4]] = -1;
                        edges[edges_n*triangles[triangles_n*i+5]] = -1;
                        triangles[triangles_n*i] = -1;
                    }
                }
                int new_point_index;
                add_point(new_point_A_index, new_point_B_index, new_point_index);

                // Search for boundary edges and save them in an array.
                int boundary_len = 0;
                int16_t boundary_edges[1024];
                for (int i = 0; i < edges_len; i++) {
                    if (edges[edges_n*i] == -1) continue;
                    // Search for an incident triangle, by looking for the opposite edge (with reversed direction).
                    bool boundary = true;
                    for (int j = 0; j < edges_len; j++) {
                        if (edges[edges_n*j] == -1) continue;
                        if (   points[points_n*edges[edges_n*i]] == points[points_n*edges[edges_n*j+1]]
                            && points[points_n*edges[edges_n*i]+1] == points[points_n*edges[edges_n*j+1]+1]
                            && points[points_n*edges[edges_n*i+1]] == points[points_n*edges[edges_n*j]]
                            && points[points_n*edges[edges_n*i+1]+1] == points[points_n*edges[edges_n*j]+1]) {
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
            fprintf(stderr, ERROR_ALERT "Code should not reach this.\n");
            exit(EXIT_FAILURE);
        }

        // The polyhedra are not intersecting. Descend the simplex and compute the closest point on the CSO.
        int A_index, B_index;
        vec3 new_point;
        cso_support(dir, new_point, A_index, B_index);
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
                vec3 closest_on_poly = closest_point_on_tetrahedron_to_point(simplex[0], simplex[1], simplex[2], simplex[3], origin);
                if (DEBUG) {
                    paint_points_c(Canvas3D, &origin, 1, "tr", 50);
                    paint_points_c(Canvas3D, &closest_on_poly, 1, "tb", 50);
                }
                return false;
            }
        } else if (n == 3 && on_simplex) {
            vec3 closest_on_poly = closest_point_on_triangle_to_point(simplex[0], simplex[1], simplex[2], origin);
            if (DEBUG) {
                paint_points_c(Canvas3D, &origin, 1, "tr", 50);
                paint_points_c(Canvas3D, &closest_on_poly, 1, "tb", 50);
            }
            return false;
        } else if (n == 2 && on_simplex) {
            vec3 closest_on_poly = closest_point_on_line_segment_to_point(simplex[0], simplex[1], origin);
            if (DEBUG) {
                paint_points_c(Canvas3D, &origin, 1, "tr", 50);
                paint_points_c(Canvas3D, &closest_on_poly, 1, "tb", 50);
            }
            return false;
        } else if (n == 1 && on_simplex) {
            if (DEBUG) {
                paint_points_c(Canvas3D, &origin, 1, "tr", 50);
                paint_points_c(Canvas3D, &simplex[0], 1, "tb", 50);
            }
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
#undef DEBUG
}
