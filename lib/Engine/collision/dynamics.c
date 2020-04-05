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

            // va,vb: The linear velocities of the rigid bodies.
            vec3 va = vec3_mul(A->linear_momentum, A->inverse_mass);
            vec3 vb = vec3_mul(B->linear_momentum, B->inverse_mass);
            // wa,wb: The angular velocities of the rigid bodies.
            vec3 wa = matrix_vec3(A->inverse_inertia_tensor, A->angular_momentum);
            vec3 wb = matrix_vec3(B->inverse_inertia_tensor, B->angular_momentum);
            // If the bodies are colliding, manifold will contain contact information.
            GJKManifold manifold;
            bool colliding = convex_hull_intersection(A->shape.polytope.points, A->shape.polytope.num_points, &A_matrix,
                                                      B->shape.polytope.points, B->shape.polytope.num_points, &B_matrix, &manifold);
            if (colliding) {
                // n: The normalized direction of separation.
                vec3 n = vec3_normalize(manifold.separating_vector);
                // a_pos,b_pos: The relative positions of the points of contact of A and B.
                vec3 a_pos = vec3_sub(manifold.A_closest, Transform_position(t));
                vec3 b_pos = vec3_sub(manifold.B_closest, Transform_position(t2));
                // dpa,dpb: The linear velocities at the points of contact.
                vec3 dpa = vec3_add(va, vec3_cross(wa, a_pos));
                vec3 dpb = vec3_add(vb, vec3_cross(wb, b_pos));
                // relative_velocity: If the point of contact of A were hitting B in a frame where B is still, then this is A's velocity.
                //                    This is needed to calculate the force required for "bouncing A off of B".
                vec3 relative_velocity = vec3_sub(dpa, dpb);
                if (vec3_dot(relative_velocity, n) > 0) {
                    // Colliding contact.

                    // Separate the objects.
                    // Move each object away from each other along the separating vector, taking into account the relative momentums at the points of contact.
                    // Calculate the linear momentums at the points of contact.
                    vec3 momentum_pa = vec3_add(A->linear_momentum, vec3_cross(a_pos, A->angular_momentum));
                    vec3 momentum_pb = vec3_add(B->linear_momentum, vec3_cross(b_pos, B->angular_momentum));
                    float dpan = vec3_dot(dpa, n);
                    float dpbn = vec3_dot(dpb, n);
                    float momentum_pan = vec3_dot(momentum_pa, n);
                    float momentum_pbn = vec3_dot(momentum_pb, n);
                    if (momentum_pan - momentum_pbn < 0.001) {
                        Transform_move(t, vec3_mul(vec3_neg(manifold.separating_vector), 1.1)); //---Offset because resting contact crashes. Seriously need to work on robustness.
                    } else {
                        float a_sep = momentum_pan / (momentum_pan - momentum_pbn);
                        float b_sep = momentum_pbn / (momentum_pan - momentum_pbn);
                        Transform_move(t, vec3_mul(manifold.separating_vector, -a_sep * 1.1));
                        Transform_move(t2, vec3_mul(manifold.separating_vector, b_sep * 1.1));
                    }

                    float coefficient_of_restitution = 0.5;
                    float numerator = -(1 + coefficient_of_restitution) * (vec3_dot(relative_velocity, n) + vec3_dot(wa, vec3_cross(a_pos, n)) - vec3_dot(wb, vec3_cross(b_pos, n)));
                    float denominator = A->inverse_mass + B->inverse_mass + vec3_dot(vec3_cross(a_pos, n), matrix_vec3(A->inverse_inertia_tensor, vec3_cross(a_pos, n)))
                                                                          + vec3_dot(vec3_cross(b_pos, n), matrix_vec3(B->inverse_inertia_tensor, vec3_cross(b_pos, n)));
                    if (denominator == 0) {
                        fprintf(stderr, ERROR_ALERT "Denominator in calculation of impulse magnitude between rigid bodies should never be zero.\n");
                        exit(EXIT_FAILURE);
                    }
                    float impulse_magnitude = numerator / denominator;
                    //printf("impulse_magnitude: %.6f\n", impulse_magnitude);
                    //printf("A mass: %.6f\n", A->mass);
                    //printf("B mass: %.6f\n", B->mass);
                    //printf("A inertia tensor:\n");
                    //print_matrix3x3f(&A->inertia_tensor);
                    //printf("A inverse inertia tensor:\n");
                    //print_matrix3x3f(&A->inverse_inertia_tensor);
                    //printf("B inertia tensor:\n");
                    //print_matrix3x3f(&B->inertia_tensor);
                    //printf("B inverse inertia tensor:\n");
                    //print_matrix3x3f(&B->inverse_inertia_tensor);
                    //getchar();
                    vec3 impulse_force = vec3_mul(n, impulse_magnitude);
                    A->linear_momentum = vec3_add(A->linear_momentum, impulse_force);
                    B->linear_momentum = vec3_sub(B->linear_momentum, impulse_force);
                    A->angular_momentum = vec3_add(A->linear_momentum, vec3_mul(vec3_cross(a_pos, n), impulse_magnitude));
                    B->angular_momentum = vec3_sub(B->linear_momentum, vec3_mul(vec3_cross(b_pos, n), impulse_magnitude));
                }
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

        // Calculate the angular velocity from angular momentum and the (inverse) inertia tensor.
        vec3 angular_velocity = matrix_vec3(rb->inverse_inertia_tensor, rb->angular_momentum);
        float dwx,dwy,dwz;
        dwx = angular_velocity.vals[0] * dt;
        dwy = angular_velocity.vals[1] * dt;
        dwz = angular_velocity.vals[2] * dt;
        mat3x3 skew;
        fill_mat3x3(skew, 0,   -dwz,  dwy,
                          dwz,    0, -dwx,
                          -dwy, dwx,    0);
        t->rotation_matrix = mat3x3_add(t->rotation_matrix, multiply_mat3x3(skew, t->rotation_matrix));
        // Orthonormalize to prevent matrix drift.
        mat3x3_orthonormalize(&t->rotation_matrix);
    end_for_aspect()
}

void rigid_body_dynamics(void)
{
    update_rigid_bodies();
    resolve_rigid_body_collisions();
}
