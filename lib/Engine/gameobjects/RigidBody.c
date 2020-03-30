/*================================================================================
    RigidBody
================================================================================*/
#include "Engine.h"

AspectType RigidBody_TYPE_ID;

void RigidBody_init_polyhedron(RigidBody *rb, Polyhedron poly, float mass)
{
    rb->type = RigidBodyPolyhedron;
    rb->shape.polyhedron = poly;
    rb->mass = mass;
    rb->inverse_mass = mass == 0 ? 0 : 1.0 / mass;
    
    Transform *transform = other_aspect(rb, Transform);
    // Calculate the center of mass (assuming the mass is uniform).
    PolyhedronTriangle *tri = poly.triangles.first;

    printf("calculating ...\n");
    vec3 center_of_mass = new_vec3(0,0,0);

    float volume_times_6 = 0;
    while (tri != NULL) {
        // Calculate the centroid of the tetrahedron containing the origin and the points of the triangle.
        vec3 centroid = vec3_mul(vec3_add(vec3_add(tri->points[0]->position, tri->points[1]->position), tri->points[2]->position), 0.25);
        // paint_points_c(Canvas3D, &centroid, 1, "k", 20);
        // Calculate the volume of this same tetrahedron.
        float v = tetrahedron_6_times_volume(new_vec3(0,0,0), tri->points[0]->position, tri->points[1]->position, tri->points[2]->position);
        volume_times_6 += v;
        center_of_mass = vec3_add(center_of_mass, vec3_mul(centroid, v));
        tri = tri->next;
    }
    center_of_mass = vec3_mul(center_of_mass, 1.0 / volume_times_6);
    print_vec3(center_of_mass);
    rb->center_of_mass = center_of_mass;
    // Update the transform center. This is by default (0,0,0), but the center can be changed to make adjustments to the transform matrix.
    // This is useful because then geometry (in application or in vram) does not need to be changed for a change of center of rotation.
    transform->center = center_of_mass;

    // pause();
}

