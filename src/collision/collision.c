/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

vec3 *random_points(float radius, int n)
{
    // So the "random" convex polyhedra have some sort of variety, use Gram-Schmidt to create a semi-random orthonormal basis, and
    // then have r1,r2,r3 be the "random principal axes", that weight the points in each of those directions.

    float size = frand()*1.9 + 0.1; // size multiplies the base radius.

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
        points[i] = vec3_mul(vec3_add(vec3_add(vec3_mul(e1, frand()-0.5), vec3_mul(e2, frand()-0.5)), vec3_mul(e3, frand()-0.5)), radius * size);
        points[i] = vec3_add(points[i], offset);
        print_vec3(points[i]);
    }
    return points;
}

vec3 *points;
Polyhedron hull;
bool viewing = false;

void create_object(void)
{
    int min = 6;
    int max = 100;
    int n = (rand() % (max - min)) + min;
    points = random_points(50, n);
    //-------destroy the previous hull polyhedron.
    Polyhedron poly = convex_hull(points, n);
    printf("%d\n", polyhedron_num_points(&poly));

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

    printf("%d vertices, %d triangles\n", mesh.num_vertices, mesh.num_triangles);
    
    EntityID e = new_entity(4);
    float d = 300;
    Transform *transform = add_aspect(e, Transform);
    Transform_set(transform, frand()*d-d/2, frand()*d-d/2, frand()*d-d/2, 0,0,0);
    //Transform_set(add_aspect(e, Transform), 0,0,0,0,0,0);
    Body *b = add_aspect(e, Body);
    b->visible = true;
    transform->scale = 1;
    Geometry *g = oneoff_resource(Geometry, b->geometry);
    *g = geometry;
    //b->material = Material_create("Materials/red");
    //b->material = Material_create("Materials/textured_phong_shadows_phong_tessellation");
    b->material = Material_create("Materials/textured_phong_shadows");
    material_set_texture_path(resource_data(Material, b->material), "diffuse_map", "Textures/minecraft/dirt");

    RigidBody *rb = add_aspect(e, RigidBody);
    RigidBody_init_polyhedron(rb, poly, 1);
    float s = 30;
    rb->linear_momentum = new_vec3(frand()*s-s/2,frand()*s-s/2,frand()*s-s/2);
    float r = 1;
    rb->angular_velocity = new_vec3(frand()*r-r/2,frand()*r-r/2,frand()*r-r/2);

    hull = poly;
}


extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_P) {
            create_object();
        }
        if (key == GLFW_KEY_V) viewing = !viewing;
    }
}
    
extern void mouse_button_event(int button, int action, int mods)
{
}
extern void cursor_move_event(double x, double y)
{
}


Polyhedron bunny_hull;
mat4x4 bunny_matrix;
extern void init_program(void)
{
    test_directional_light_controlled();
    // open_scene(g_scenes, "block_on_floor");
    create_key_camera_man(0,0,0,  0,0,0);
    create_object();

    EntityID e = new_entity(4);
    Transform_set(add_aspect(e, Transform), -500,-1190,-500,0,0,0);
    Body *body = add_aspect(e, Body);
    get_aspect_type(e, Transform)->scale = 1000;
    body->visible = true;
    body->geometry = new_resource_handle(Geometry, "Models/block");
    body->material = Material_create("Materials/textured_phong_shadows");
    body->is_ground = true;
    material_set_texture_path(resource_data(Material, body->material), "diffuse_map", "Textures/marble_tile");

    int bunny_square_root = 1;
    for (int i = 0; i < bunny_square_root; i++) {
        for (int j = 0; j < bunny_square_root; j++) {
            EntityID e = new_entity(4);
            float apart = 400;
            Transform *t = add_aspect(e, Transform);
            Transform_set(t, 200+i*apart,-250,200+j*apart, 0,0,0);
            Body *body = add_aspect(e, Body);
            t->scale = 1870;
            body->visible = true;
            body->geometry = new_resource_handle(Geometry, "Models/stanford_bunny -a");
            body->material = Material_create("Materials/textured_phong_shadows");
            material_set_texture_path(resource_data(Material, body->material), "diffuse_map", "Textures/minecraft/stone_bricks");
            MeshData *mesh = resource_data(Geometry, body->geometry)->mesh_data;
            bunny_hull = convex_hull(mesh->attribute_data[Position], mesh->num_vertices);
            bunny_matrix = Transform_matrix(t);
        }
    }
}
extern void loop_program(void)
{
    draw_polyhedron(&hull, NULL);
    draw_polyhedron_winding_order(&hull, "k", 10, NULL);
    draw_polyhedron(&bunny_hull, &bunny_matrix);
    draw_polyhedron_winding_order(&bunny_hull, "k", 10, &bunny_matrix);
    for_aspect(RigidBody, rb)
        Body *b = other_aspect(rb, Body);
        if (viewing) {
            Transform *t = other_aspect(rb, Transform);
            vec3 p = Transform_position(t);
            paint_points_c(Canvas3D, &p, 1, "b", 12);
            // PolyhedronTriangle *tri = rb->shape.polyhedron.triangles.first;
            // while (tri != NULL) {
            //     for (int i = 0; i < 3; i++) paint_line_cv(Canvas3D, tri->points[i]->position, tri->points[(i+1)%3]->position, "k", 5);
            //     tri = tri->next;
            // }
            b->visible = false;
        } else {
            b->visible = true;
        }
    end_for_aspect()


}
extern void close_program(void)
{
}
