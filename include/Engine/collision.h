#ifndef HEADER_DEFINED_COLLISION
#define HEADER_DEFINED_COLLISION

/*================================================================================
    Collision.
================================================================================*/
typedef struct GJKManifold_s {
    vec3 separating_vector;
} GJKManifold;
bool convex_hull_intersection(vec3 *A, int A_len, vec3 *B, int B_len, GJKManifold *manifold);

// Debugging and visualization.
Polyhedron compute_minkowski_difference(Polyhedron A, Polyhedron B);

/*================================================================================
    Dynamics.
================================================================================*/
void rigid_body_dynamics(void);

#endif // HEADER_DEFINED_COLLISION
