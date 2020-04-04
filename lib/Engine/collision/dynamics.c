#include "Engine.h"


static void resolve_rigid_body_collisions(void)
{
    for_aspect(RigidBody, A)
        if (A->type != RigidBodyPolytope) continue;
        Transform *t = other_aspect(A, Transform);
        mat4x4 A_matrix = Transform_matrix(t);

        for_aspect(RigidBody, B)
            if (B->type != RigidBodyPolytope) continue;
            Transform *t2 = other_aspect(B, Transform);
            if (t == t2) continue;
            mat4x4 B_matrix = Transform_matrix(t2);

            GJKManifold manifold;

            vec3 va = vec3_mul(A->linear_momentum, A->inverse_mass);
            vec3 vb = vec3_mul(B->linear_momentum, B->inverse_mass);
            vec3 relative_velocity = vec3_sub(vb, va);
            bool colliding = convex_hull_intersection(A->shape.polytope.points, A->shape.polytope.num_points, &A_matrix,
                                                      B->shape.polytope.points, B->shape.polytope.num_points, &B_matrix, &manifold);
        
            if (colliding) {
                // Move each object away from each other along the separating vector, taking into account the relative momentums at the point of contact.

                // Transform_move(t, vec3_mul(vec3_neg(manifold.separating_vector), 1.1)); //---Offset because resting contact crashes. Seriously need to work on robustness.
            }
            // Polyhedron hull_A = convex_hull(A->shape.polytope.points,A->shape.polytope.num_points);
            // Polyhedron hull_B = convex_hull(B->shape.polytope.points,B->shape.polytope.num_points);
            // Polyhedron mink = compute_minkowski_difference(hull_A, hull_B);
            // draw_polyhedron2(&mink, NULL, "tk", 3);
    
        end_for_aspect()
    end_for_aspect()
}

static void update_rigid_bodies(void)
{
    for_aspect(RigidBody, rb)
        // Euler's method updating for rigid body transforms.
        Transform *t = other_aspect(rb, Transform);
        t->x += rb->linear_momentum.vals[0] * rb->inverse_mass * dt;
        t->y += rb->linear_momentum.vals[1] * rb->inverse_mass * dt;
        t->z += rb->linear_momentum.vals[2] * rb->inverse_mass * dt;

        //---calculate the angular velocity from angular momentum and the inertia tensor.

        float dwx,dwy,dwz;
        dwx = rb->angular_velocity.vals[0] * dt;
        dwy = rb->angular_velocity.vals[1] * dt;
        dwz = rb->angular_velocity.vals[2] * dt;
        mat3x3 skew;
        fill_mat3x3(skew, 0,   -dwz,  dwy,
                          dwz,    0, -dwx,
                          -dwy, dwx,    0);

        t->rotation_matrix = mat3x3_add(t->rotation_matrix, multiply_mat3x3(skew, t->rotation_matrix));
        mat3x3_orthonormalize(&t->rotation_matrix);
    end_for_aspect()
}

void rigid_body_dynamics(void)
{
    resolve_rigid_body_collisions();
    update_rigid_bodies();
}
