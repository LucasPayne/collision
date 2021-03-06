/*================================================================================
    RigidBody
================================================================================*/
#include "Engine.h"
#include "math.h"

AspectType RigidBody_TYPE_ID;

mat3x3 brute_force_polyhedron_inertia_tensor(Polyhedron poly, vec3 center, float mass)
{
    //printf("Brute forcing the inertia tensor ...\n");
    //printf("center: "); print_vec3(center);
    //printf("mass: %.2f\n", mass);
    
    float integrals[6] = {0}; // x^2, y^2, z^2, xy, xz, yz
    float volume = 0;

    PolyhedronPoint *point = poly.points.first;
    vec3 min = point->position;
    vec3 max = point->position;
    while (point != NULL) {
        for (int i = 0; i < 3; i++) {
            if (point->position.vals[i] < min.vals[i]) min.vals[i] = point->position.vals[i];
            if (point->position.vals[i] > max.vals[i]) max.vals[i] = point->position.vals[i];
        }
        point = point->next;
    }
    //print_vec3(min);
    //print_vec3(max);
    float d = 2.5;
    float dcubed = d*d*d;
    //printf("Doing ~%.0f evaluations ...\n", (max.vals[0] - min.vals[0])/d * (max.vals[1] - min.vals[1])/d * (max.vals[2] - min.vals[2])/d);

    for (float x = min.vals[0]; x <= max.vals[0]; x += d) {
        for (float y = min.vals[1]; y <= max.vals[1]; y += d) {
            for (float z = min.vals[2]; z <= max.vals[2]; z += d) {
                if (!point_in_convex_polyhedron(new_vec3(x,y,z), poly)) continue;
                volume += dcubed;
            }
        }
    }

    float inverse_volume = volume == 0 ? 0 : 1.0 / volume;
    float element = dcubed * inverse_volume;
    for (float x = min.vals[0]; x <= max.vals[0]; x += d) {
        for (float y = min.vals[1]; y <= max.vals[1]; y += d) {
            for (float z = min.vals[2]; z <= max.vals[2]; z += d) {
                if (!point_in_convex_polyhedron(new_vec3(x,y,z), poly)) continue;
                float xc = x - center.vals[0];
                float yc = y - center.vals[1];
                float zc = z - center.vals[2];
                integrals[0] += xc*xc * dcubed;
                integrals[1] += yc*yc * dcubed;
                integrals[2] += zc*zc * dcubed;
                integrals[3] += xc*yc * dcubed;
                integrals[4] += xc*zc * dcubed;
                integrals[5] += yc*zc * dcubed;
            }
        }
    }
    //printf("Volume: %.2f\n", volume);
    //printf("Volume?: %.2f\n", polyhedron_volume(poly));
    mat3x3 inertia_tensor;
    fill_mat3x3_rmaj(inertia_tensor, integrals[1]+integrals[2], -integrals[3], -integrals[4],
                                 -integrals[3], integrals[0]+integrals[2], -integrals[5],
                                 -integrals[4], -integrals[5], integrals[0]+integrals[1]);
    for (int i = 0; i < 9; i++) {
        inertia_tensor.vals[i] *= inverse_volume * mass;
    }
    //print_matrix3x3f(&inertia_tensor);
    //getchar();
    return inertia_tensor;
}

mat3x3 polyhedron_inertia_tensor(Polyhedron poly, vec3 center, float mass)
{
    // The inertia tensor is the integral over the mass (here density = 1) of
    // density * -skew(r)^2, where r is the position relative to the center. Taking the definition of
    // angular momentum as the integral over the mass of cross(r, v), and v being cross(w, r), the inertia tensor
    // is derived as the matrix-multiply relationship between the angular velocity and the angular momentum.
    //=====================================================================================
    //                           [ y^2 + z^2     -xy          -xz    ]
    // inertia tensor = integral [ -xy        x^2 + z^2       -yz    ] over the polyhedron.
    //                           [ -xz           -yz       x^2 + y^2 ]
    //=====================================================================================
    // The computation here is done by reducing the integral into lower dimensional integrals, using the divergence theorem and Green's theorem.

    printf("Computing with reduced integral method ...\n");

    float integrals[6] = {0}; // x^2, y^2, z^2, xy, xz, yz
    float volume = polyhedron_volume(poly);

    PolyhedronTriangle *tri = poly.triangles.first;
    while (tri != NULL) {
        // Compute the outward-pointing unit normal.
        vec3 n = vec3_normalize(vec3_cross(vec3_sub(tri->points[1]->position, tri->points[0]->position), vec3_sub(tri->points[2]->position, tri->points[0]->position)));
        float nx,ny,nz;
        nx = n.vals[0]; ny = n.vals[1]; nz = n.vals[2];

        for (int i = 0; i < 3; i++) {
            vec3 e1 = vec3_sub(tri->points[i]->position, center);
            vec3 e2 = vec3_sub(tri->points[(i+1)%3]->position, center);
            float e1x, e1y, e1z, e2x, e2y, e2z;
            e1x = e1.vals[0]; e1y = e1.vals[1]; e1z = e1.vals[2];
            e2x = e2.vals[0]; e2y = e2.vals[1]; e2z = e2.vals[2];

            // Form the outward unit normal of this edge coplanar with the triangle.
            vec3 m = vec3_normalize(vec3_cross(vec3_sub(e2, e1), n));
            float mx,my,mz;
            mx = m.vals[0]; my = m.vals[1]; mz = m.vals[2];

            float edge_length = vec3_length(vec3_sub(e2, e1));
            
            integrals[0] += nx * mx * (e2x*e2x*e2x*e2x*e2x - e1x*e1x*e1x*e1x*e1x) * edge_length * 1.0 / 60.0;
            integrals[1] += ny * my * (e2y*e2y*e2y*e2y*e2y - e1y*e1y*e1y*e1y*e1y) * edge_length * 1.0 / 60.0;
            integrals[2] += nz * mz * (e2z*e2z*e2z*e2z*e2z - e1z*e1z*e1z*e1z*e1z) * edge_length * 1.0 / 60.0;

        }
        tri = tri->next;
    }
    printf("volume: %.2f\n", volume);
    float inverse_volume = 1.0 / volume;
    for (int i = 0; i < 6; i++) {
        integrals[i] *= inverse_volume * mass;
    }
    mat3x3 inertia_tensor;
    fill_mat3x3_rmaj(inertia_tensor, integrals[1]+integrals[2], -integrals[3], -integrals[4],
                                 -integrals[3], integrals[0]+integrals[2], -integrals[5],
                                 -integrals[4], -integrals[5], integrals[0]+integrals[1]);
    //print_matrix3x3f(&inertia_tensor);
    //getchar();
    return inertia_tensor;
}

void RigidBody_init_polytope(RigidBody *rb, vec3 *points, int num_points, float mass)
{
    rb->type = RigidBodyPolytope;
    rb->shape.polytope.points = points;
    rb->shape.polytope.num_points = num_points;
    rb->mass = mass;
    rb->inverse_mass = mass == 0 ? 0 : 1.0 / mass;
    
    vec3 center_of_mass = polytope_center_of_mass(points, num_points);

    print_vec3(center_of_mass);
    rb->center_of_mass = center_of_mass;
    // Update the transform center. This is by default (0,0,0), but the center can be changed to make adjustments to the transform matrix.
    // This is useful because then geometry (in application or in vram) does not need to be changed for a change of center of rotation.
    Transform *transform = other_aspect(rb, Transform);
    transform->center = center_of_mass;

    mat3x3 inertia_tensor = brute_force_polyhedron_inertia_tensor(convex_hull(points, num_points), center_of_mass, mass);
    if (mass == 0) {
        memset(&rb->inertia_tensor, 0, sizeof(mat3x3));
        memset(&rb->inverse_inertia_tensor, 0, sizeof(mat3x3));
    } else {
        rb->inertia_tensor = inertia_tensor;
        rb->inverse_inertia_tensor = mat3x3_inverse(inertia_tensor);
    }
    //print_matrix3x3f(&rb->inertia_tensor);
    //print_matrix3x3f(&rb->inverse_inertia_tensor);
    //right_multiply_mat3x3(&inertia_tensor, &rb->inverse_inertia_tensor);
    //print_matrix3x3f(&inertia_tensor);
    //getchar();
    // polyhedron_inertia_tensor(poly, center_of_mass, mass);
}

