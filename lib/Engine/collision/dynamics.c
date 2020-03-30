#include "Engine.h"

void rigid_body_dynamics(void)
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
