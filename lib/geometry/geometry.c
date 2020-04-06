#include "geometry.h"

#define ABS(X) ((X) < 0 ? -(X) : (X))

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
void ___dl_remove(DLList *list, DLNode *node)
{
    if (node == list->first && node == list->last) {
        list->first = NULL;
        list->last = NULL;
    } else if (node == list->first) {
        list->first = list->first->next;
        list->first->prev = NULL;
    } else if (node == list->last) {
        list->last = list->last->prev;
        list->last->next = NULL;
    } else {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }
    free(node);
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

//--------------------------------------------------------------------------------
// Closest-points methods.
//--------------------------------------------------------------------------------
vec3 closest_point_on_line_to_point(vec3 a, vec3 b, vec3 p)
{
    // This is an unlimited line.
    vec3 ab = vec3_sub(b, a);
    vec3 ap = vec3_sub(p, a);
    return vec3_add(a, vec3_mul(ab, vec3_dot(ap, ab) / vec3_dot(ab, ab)));
}
vec3 closest_point_on_line_segment_to_point(vec3 a, vec3 b, vec3 p)
{
    if (vec3_dot(vec3_sub(p, a), vec3_sub(b, a)) < 0) {
        return a;
    }
    if (vec3_dot(vec3_sub(p, b), vec3_sub(a, b)) < 0) {
        return b;
    }
    return closest_point_on_line_to_point(a, b, p);
}

vec3 barycentric_triangle(vec3 a, vec3 b, vec3 c, float wa, float wb, float wc)
{
    return vec3_mul(vec3_add(vec3_mul(a, wa), vec3_add(vec3_mul(b, wb), vec3_mul(c, wc))), 1.0/(wa + wb + wc));
}

// Get the barycentric coordinates of the projection of the point into the plane spanned by the triangle.
vec3 point_to_triangle_plane_barycentric(vec3 a, vec3 b, vec3 c, vec3 p)
{
    //---all three collinear gives a divison by zero?
    vec3 n = vec3_cross(vec3_sub(b, a), vec3_sub(c, a));
    vec3 ap = vec3_sub(p, a);
    vec3 bp = vec3_sub(p, b);
    vec3 cp = vec3_sub(p, c);
    vec3 w;
    w.vals[0] = vec3_dot(vec3_cross(bp, cp), n);
    w.vals[1] = vec3_dot(vec3_cross(cp, ap), n);
    w.vals[2] = vec3_dot(vec3_cross(ap, bp), n);
    float winv = 1.0 / (w.vals[0] + w.vals[1] + w.vals[2]);
    w.vals[0] *= winv; w.vals[1] *= winv; w.vals[2] *= winv;
    return w;
}
// Get the cartesian coordinates of the projection of the point into the plane spanned by the triangle.
vec3 point_to_triangle_plane(vec3 a, vec3 b, vec3 c, vec3 p)
{
    vec3 w = point_to_triangle_plane_barycentric(a, b, c, p);
    return barycentric_triangle(a, b, c, w.vals[0], w.vals[1], w.vals[2]);
}

vec3 closest_point_on_triangle_to_point(vec3 a, vec3 b, vec3 c, vec3 p)
{
    // Get the barycentric coordinates of the projection of p into the triangle's plane,
    // then use barycentric regions then tests against normals to determine the Voronoi region,
    // and return the closest point on the relevant feature.

    vec3 w = point_to_triangle_plane_barycentric(a, b, c, p);
    float wa,wb,wc;
    wa = w.vals[0]; wb = w.vals[1]; wc = w.vals[2];
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

// Order: a,b,c,d,  p
// This is for strict inclusion, not on the boundary.
bool point_in_tetrahedron(vec3 a, vec3 b, vec3 c, vec3 d, vec3 p)
{
    float wa = tetrahedron_6_times_volume(a, b, c, p);
    float wb = tetrahedron_6_times_volume(b, a, d, p);
    float wc = tetrahedron_6_times_volume(c, b, d, p);
    float wd = tetrahedron_6_times_volume(a, c, d, p);
    // Return whether or not the weights all have the same sign.
    return (wa != 0 && wa < 0 == wb < 0 && wb < 0 == wc < 0 && wc < 0 == wd < 0);
}
vec3 closest_point_on_tetrahedron_to_point(vec3 a, vec3 b, vec3 c, vec3 d, vec3 p)
{
    // This method just takes all the closest points on each triangle, and takes the one of minium distance, unless the point is inside the tetrahedron.
    // This could definitely be better.
    if (point_in_tetrahedron(a,b,c,d, p)) return p;

    vec3 close_points[4];
    close_points[0] = closest_point_on_triangle_to_point(a,b,c, p);
    close_points[1] = closest_point_on_triangle_to_point(a,b,d, p);
    close_points[2] = closest_point_on_triangle_to_point(a,c,d, p);
    close_points[3] = closest_point_on_triangle_to_point(b,c,d, p);
    float mindis = -1;
    int min_index = 0;
    for (int i = 0; i < 4; i++) {
        float dis = vec3_dot(vec3_sub(close_points[i], p), vec3_sub(close_points[i], p));
        if (mindis < 0 || dis < mindis) {
            mindis = dis; min_index = i;
        }
    }
    return close_points[min_index];
}

//--------------------------------------------------------------------------------
// Simplex methods.
//--------------------------------------------------------------------------------
#define bad_simplex_error() {\
    fprintf(stderr, ERROR_ALERT "simplex method: Bad input. Input must be a simplex of 1,2,3, or 4 vertices.\n");\
    exit(EXIT_FAILURE);\
}
vec3 closest_point_on_simplex(int n, vec3 points[], vec3 p)
{
    if (n == 1) return points[0];
    if (n == 2) return closest_point_on_line_segment_to_point(points[0], points[1], p);
    if (n == 3) return closest_point_on_triangle_to_point(points[0], points[1], points[2], p);
    if (n == 4) return closest_point_on_tetrahedron_to_point(points[0], points[1], points[2], points[3], p);
    bad_simplex_error();
}
int simplex_extreme_index(int n, vec3 points[], vec3 dir)
{
    // Returns the index of a support vector in the simplex vertices in the given direction.
    if (n < 1 || n > 4) bad_simplex_error();
    float dist = vec3_dot(points[0], dir);
    int index = 0;
    for (int i = 1; i < n; i++) {
        float new_dist = vec3_dot(points[i], dir);
        if (new_dist > dist) {
            index = i;
            dist = new_dist;
        }
    }
    return index;
}


/*--------------------------------------------------------------------------------
    Intersection methods.
--------------------------------------------------------------------------------*/

// Ray-triangle methods and variants.
//--------------------------------------------------------------------------------
// Test whether the weights are a convex combination of the triangle points.
#define barycentric_triangle_convex(WA,WB,WC)\
    (0 <= ( WA ) && ( WA ) <= 1 && 0 <= ( WB ) && ( WB ) <= 1 && 0 <= ( WC ) && ( WC ) <= 1)
#define barycentric_triangle_convex_v(W)\
    barycentric_triangle_convex(( W ).vals[0], ( W ).vals[1], ( W ).vals[2])
    
// Get the intersection of the ray with the plane the triangle defines, in barycentric coordinates.
bool ray_triangle_plane_intersection_barycentric(vec3 origin, vec3 direction, vec3 a, vec3 b, vec3 c, vec3 *intersection)
{
    float wa = vec3_dot(direction, vec3_cross(vec3_sub(b, origin), vec3_sub(c, origin)));
    float wb = vec3_dot(direction, vec3_cross(vec3_sub(c, origin), vec3_sub(a, origin)));
    float wc = vec3_dot(direction, vec3_cross(vec3_sub(a, origin), vec3_sub(b, origin)));
    const float epsilon = 0.001;
    float w = wa + wb + wc;
    if (ABS(w) < epsilon || vec3_dot(vec3_sub(barycentric_triangle(a,b,c, wa,wb,wc), origin), direction) < 0) return false;
    float winv = 1.0 / w;
    wa *= winv;
    wb *= winv;
    wc *= winv;
    *intersection = new_vec3(wa,wb,wc);
    return true;
}
// Give the intersection as cartesian coordinates.
bool ray_triangle_plane_intersection(vec3 origin, vec3 direction, vec3 a, vec3 b, vec3 c, vec3 *intersection)
{
    vec3 inter;
    if (!ray_triangle_plane_intersection_barycentric(origin, direction, a, b, c, &inter)) return false;
    *intersection = barycentric_triangle_v(a,b,c, inter);
    return true;
}
// Get the intersection of the ray with the triangle, in barycentric coordinates,
bool ray_triangle_intersection_barycentric(vec3 origin, vec3 direction, vec3 a, vec3 b, vec3 c, vec3 *intersection)
{
    vec3 weights;
    if (!ray_triangle_plane_intersection_barycentric(origin, direction, a, b, c, &weights)) return false;
    if (!barycentric_triangle_convex_v(weights)) return false;
    *intersection = weights;
    return true;
}
// Give the intersection as cartesian coordinates.
bool ray_triangle_intersection(vec3 origin, vec3 direction, vec3 a, vec3 b, vec3 c, vec3 *intersection)
{
    vec3 weights;
    if (!ray_triangle_plane_intersection_barycentric(origin, direction, a, b, c, &weights)) return false;
    if (!barycentric_triangle_convex_v(weights)) return false;
    *intersection = barycentric_triangle_v(a,b,c, weights);
    return true;
}

