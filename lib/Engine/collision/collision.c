/*================================================================================
================================================================================*/
#include "Engine.h"

static vec3 closest_point_on_line_to_point(vec3 a, vec3 b, vec3 p)
{
    // This is an unlimited line.
    vec3 ab = vec3_sub(b, a);
    vec3 ap = vec3_sub(p, a);
    return vec3_add(a, vec3_mul(ab, vec3_dot(ap, ab) / vec3_dot(ab, ab)));
}

static vec3 barycentric_triangle(vec3 a, vec3 b, vec3 c, float wa, float wb, float wc)
{
    return vec3_mul(vec3_add(vec3_mul(a, wa), vec3_add(vec3_mul(b, wb), vec3_mul(c, wc))), 1.0/(wa + wb + wc));
}
static bool barycentric_triangle_convex(float wa, float wb, float wc)
{
    // tests whether the weights wa+wb+wc = 1 are a convex combination of the triangle points.
    // Weights _must_ have wa+wb+wc for this to work.
    return 0 <= wa && wa <= 1 && 0 <= wb && wb <= 1 && 0 <= wc && wc <= 1;
}
static vec3 closest_point_on_triangle_to_point(vec3 a, vec3 b, vec3 c, vec3 p)
{
    vec3 n = vec3_cross(vec3_sub(b, a), vec3_sub(c, a));
    vec3 ap = vec3_sub(p, a);
    vec3 bp = vec3_sub(p, b);
    vec3 cp = vec3_sub(p, c);
    float wa = vec3_dot(vec3_cross(bp, cp), n);
    float wb = vec3_dot(vec3_cross(cp, ap), n);
    float wc = vec3_dot(vec3_cross(ap, bp), n);
    float winv = 1.0 / (wa + wb + wc);
    wa *= winv; wb *= winv; wc *= winv;
    if (wa < 0) {
       if (vec3_dot(vec3_sub(p, b), vec3_sub(c, b)) < 0) return b;
       if (vec3_dot(vec3_sub(p, c), vec3_sub(b, c)) < 0) return c;
       return closest_point_on_line_to_point(b, c, p);
    }
    if (wb < 0) {
       if (vec3_dot(vec3_sub(p, c), vec3_sub(a, c)) < 0) return c;
       if (vec3_dot(vec3_sub(p, a), vec3_sub(c, a)) < 0) return a;
       return closest_point_on_line_to_point(c, a, p);
    }
    if (wc < 0) {
        if (vec3_dot(vec3_sub(p, a), vec3_sub(b, a)) < 0) return a;
        if (vec3_dot(vec3_sub(p, b), vec3_sub(a, b)) < 0) return b;
        return closest_point_on_line_to_point(a, b, p);
    }
    return barycentric_triangle(a,b,c, wa,wb,wc);
}

static vec3 closest_point_on_simplex(int n, vec3 points[], vec3 p)
{
    if (n == 1) return points[0];
    if (n == 2) {
        if (vec3_dot(vec3_sub(p, points[0]), vec3_sub(points[1], points[0])) < 0) {
            return points[0];
        }
        if (vec3_dot(vec3_sub(p, points[1]), vec3_sub(points[0], points[1])) < 0) {
            return points[1];
        }
        return closest_point_on_line_to_point(points[0], points[1], p);
    }
    if (n == 3) {
        return closest_point_on_triangle_to_point(points[0], points[1], points[2], p);
    }
    if (n == 4) {
        vec3 a = points[0]; vec3 b = points[1]; vec3 c = points[2]; vec3 d = points[3];
        vec3 close_points[4];
        close_points[0] = closest_point_on_triangle_to_point(a,b,c, p);
        close_points[1] = closest_point_on_triangle_to_point(a,b,d, p);
        close_points[2] = closest_point_on_triangle_to_point(a,c,d, p);
        close_points[3] = closest_point_on_triangle_to_point(b,c,d, p);
        float mindis = -1;
        int min_index = 0;
        for (int i = 0; i < 4; i++) {
            float dis = vec3_dot(close_points[i], close_points[i]);
            if (mindis < 0 || dis < mindis) {
                mindis = dis; min_index = i;
            }
        }
        return close_points[min_index];
    }
    return vec3_zero(); //bad input
}

/*
static vec3 support(vec3 *poly, int len, vec3 direction, int *index)
{
    // Assuming no structure to the convex polyhedra. Just iterates over all points and takes the maximum in the given direction.
    vec3 support_vector = poly[0];
    *index = 0;
    float support_value = vec3_dot(support_vector, direction);
    for (int i = 1; i < len; i++) {
        float new_support_value = vec3_dot(poly[i], direction);
        if (new_support_value > support_value) {
            support_value = new_support_value;
            support_vector = poly[i];
            *index = i;
        }
    }
    return support_vector;
}
static vec3 md_support(vec3 *A, int A_len, vec3 *B, int B_len, vec3 direction, int *A_index, int *B_index)
{
    vec3 A_support = support(A, A_len, direction, A_index);
    vec3 B_support = support(B, B_len, vec3_sub(0, direction), B_index);
    return vec3_sub(A_support, B_support);
}
bool convex_polyhedra_intersection(vec3 *A, int A_len, vec3 *B, int B_len, vec3 *separating_vector)
{
    // Gilbert-Johnson-Keerthi algorithm.
    vec3 start_direction = new_vec3(1,1,1);
    // Extreme points of the Minkowski difference are successively used to descend a simplex in search of the origin.
    // The GJK algorithm tries to find a simplex formed from points of A (-) B bounding the origin.
    vec3 simplex[4] = {0};
    // Indices are stored for the points on each polyhedra which gave these extreme points of the Minkowski difference.
    int A_indices[4] = {0};
    int B_indices[4] = {0};
    int n = 2;
    // Initialize a 2-simplex (a line segment).
    simplex[0] = md_support(A, A_len, B, B_len, start_direction, &A_indices[0], &B_indices[0]);
    simplex[1] = md_support(A, A_len, B, B_len, start_direction, &A_indices[1], &B_indices[1]);
    while (1) {
        vec3 closest_point = closest_point_on_simplex(n, simplex, vec3_zero());
        int replace_index = n; // The index to add the new simplex point at.
        if (n == 4) {
            // If the simplex is a tetrahedron, remove the point least extreme in the new direction.
            // The index of this removed point is the index to continue adding at.
            replace_index = 0;
            float sup = vec3_dot(simplex[0], closest_point);
            for (int i = 1; i < n; i++) {
                float new_sup = vec3_dot(simplex[i], closest_point);
                if (new_sup > sup) {
                    replace_index = i;
                    sup = new_sup;
                }
            }
            n --;
        }
        int A_index, B_index;
        vec3 support = md_support(A, A_len, B, B_len, vec3_sub(0, closest_point), &A_index, &B_index);
        vec3 support_value = vec3_dot(support, closest_point);
        if (support_value <= 0) {
            // The polyhedra are intersecting. ---(not returning any information about the intersection).
            return true;
        }
        // Add the new point to the simplex.
        simplex[replace_index] = support;
        A_indices[replace_index] = A_index;
        B_indices[replace_index] = B_index;
        // Check if the new point is really a new point. If it was already on the simplex then there is no intersection.
        for (int i = 0; i < n; i++) {
            if (i == replace_index) continue; // make sure not to check against the index of the new point.
            if (A_indices[i] == A_index && B_indices[i] == B_index) {
                // If both indices are the same on both polyhedra, this is the same point. ---could it be easier to just check equality of vec3s?
                // Not intersecting.
                return false;
            }
        }
    }
}
*/
