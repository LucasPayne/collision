/*--------------------------------------------------------------------------------
    Utilities for testing geometric algorithms.
--------------------------------------------------------------------------------*/
#include "Engine.h"

vec3 *random_points(float radius, int n)
{
    // So the "random" convex polyhedra have some sort of variety, use Gram-Schmidt to create a semi-random orthonormal basis, and
    // then have r1,r2,r3 be the "random principal axes", that weight the points in each of those directions.

    // float size = frand()*1.9 + 0.1; // size multiplies the base radius.
    float size = 1;

    vec3 e1, e2, e3;
    e1 = new_vec3(frand()-0.5, frand()-0.5, frand()-0.5);
    e1 = vec3_normalize(e1);
    e2 = new_vec3(frand()-0.5, frand()-0.5, frand()-0.5);
    e2 = vec3_normalize(vec3_sub(e2, vec3_mul(e1, vec3_dot(e2, e1))));
    e3 = vec3_cross(e1, e2);
    // Extend these so the random convex polyhedra is roughly ellipsoidal.
    e1 = vec3_mul(e1, 0.5 + frand());
    e2 = vec3_mul(e2, 0.5 + frand());
    e3 = vec3_mul(e3, 0.5 + frand());

    vec3 *points = malloc(sizeof(vec3) * n);
    mem_check(points);
    float o = 50;
    vec3 offset = new_vec3(frand()*o-o/2,frand()*o-o/2,frand()*o-o/2);
    for (int i = 0; i < n; i++) {
        vec3 f = vec3_normalize(rand_vec3(1));
        float r = frand();
        //points[i] = vec3_mul(vec3_add(vec3_add(vec3_mul(e1, frand()-0.5), vec3_mul(e2, frand()-0.5)), vec3_mul(e3, frand()-0.5)), radius * size);
        points[i] = vec3_mul(vec3_add(vec3_mul(e1, f.vals[0]), vec3_add(vec3_mul(e2, f.vals[1]), vec3_mul(e3, f.vals[2]))), radius * size * r);
        points[i] = vec3_add(points[i], offset);
        print_vec3(points[i]);
    }
    return points;
}

Polyhedron random_convex_polyhedron(float radius, int n)
{
    // Uses random_points, and creates the hull around those points.
    vec3 *points = random_points(radius, n);
    Polyhedron poly = convex_hull(points, n);
    free(points);
    return poly;
}

// Create a viewable entity from the given polyhedron.
//---just a testing function, subject to change/deletion.
EntityID polyhedron_create_entity(Polyhedron poly, vec3 position, char *texture_path)
{
    // Convert the polyhedron in to a MeshData struct.
    MeshData mesh;
    mesh.vertex_format = VERTEX_FORMAT_3NU;
    mesh.num_vertices = polyhedron_num_points(&poly);
    mesh.num_triangles = polyhedron_num_triangles(&poly);
    mesh.attribute_data[Position] = malloc(sizeof(float)*3*mesh.num_vertices);
    mem_check(mesh.attribute_data[Position]);
    PolyhedronPoint *p = poly.points.first;
    int pi = 0;
    while (p != NULL) {
        p->mark = pi; // mark this point with an index so the triangles can reference it by index.
        ((vec3 *) mesh.attribute_data[Position])[pi++] = p->position;
        p = p->next;
    }
    mesh.attribute_data_sizes[Position] = sizeof(float)*3*mesh.num_vertices;
    mesh.triangles = malloc(sizeof(uint32_t)*3*mesh.num_triangles);
    mem_check(mesh.triangles);
    PolyhedronTriangle *t = poly.triangles.first;
    int ti = 0;
    while (t != NULL) {
        for (int i = 0; i < 3; i++) mesh.triangles[3*ti + i] = t->points[i]->mark;
        t = t->next;
        ti ++;
    }
    MeshData_calculate_normals(&mesh);
    MeshData_calculate_uv_orthographic(&mesh, new_vec3(1,1,1), 0.1);
    Geometry geometry = upload_mesh(&mesh);
    
    EntityID e = new_entity(4);
    Transform *transform = add_aspect(e, Transform);
    Transform_set(transform, position.vals[0],position.vals[1],position.vals[2], 0,0,0);
    Body *b = add_aspect(e, Body);
    b->visible = true;
    transform->scale = 1;
    Geometry *g = oneoff_resource(Geometry, b->geometry);
    *g = geometry;
    if (texture_path != NULL) {
        b->material = Material_create("Materials/textured_phong_shadows");
        material_set_texture_path(resource_data(Material, b->material), "diffuse_map", texture_path);
    } else {
        b->material = Material_create("Materials/red");
    }
    return e;
}
