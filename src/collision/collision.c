/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"


vec3 *points;
Polyhedron hull;
bool viewing = false;

#define ROCK 0
#define ROD 1
#define PLATE 2
#define ICOSAHEDRON 3
RigidBody *create_object(int mode, vec3 position, vec3 euler_angles)
{
    int num_points;
    if (mode == ROCK) {
        int min = 6;
        int max = 100;
        num_points = (rand() % (max - min)) + min;
        points = random_points(50, num_points);
    } else if (mode == ROD) {
        points = malloc(sizeof(float)*3*8);
        mem_check(points);
        float w = 100;
        float h = 10;
        points[0] = new_vec3(-w,-h,-h);
        points[1] = new_vec3(-w,h,-h);
        points[2] = new_vec3(-w,-h,h);
        points[3] = new_vec3(-w,h,h);
        points[4] = new_vec3(w,-h,-h);
        points[5] = new_vec3(w,h,-h);
        points[6] = new_vec3(w,-h,h);
        points[7] = new_vec3(w,h,h);
        num_points = 8;
    } else if (mode == PLATE) {
        points = malloc(sizeof(float)*3*8);
        mem_check(points);
        float w = 10;
        float h = 100;
        points[0] = new_vec3(-w,-h,-h);
        points[1] = new_vec3(-w,h,-h);
        points[2] = new_vec3(-w,-h,h);
        points[3] = new_vec3(-w,h,h);
        points[4] = new_vec3(w,-h,-h);
        points[5] = new_vec3(w,h,-h);
        points[6] = new_vec3(w,-h,h);
        points[7] = new_vec3(w,h,h);
        num_points = 8;
    } else if (mode == ICOSAHEDRON) {
        Polyhedron ico = make_icosahedron(30 + frand()*70);
        points = polyhedron_points(ico);
        num_points = polyhedron_num_points(&ico);
    }
    //-------destroy the previous hull polyhedron.
    Polyhedron poly = convex_hull(points, num_points);
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
    Transform *transform = add_aspect(e, Transform);
    //float d = 300;
    //Transform_set(transform, frand()*d-d/2, frand()*d-d/2, frand()*d-d/2, 0,0,0);
    Transform_set(transform, position.vals[0],position.vals[1],position.vals[2], euler_angles.vals[0],euler_angles.vals[1],euler_angles.vals[2]);
    //Transform_set(add_aspect(e, Transform), 0,0,0,0,0,0);
    Body *b = add_aspect(e, Body);
    b->visible = true;
    transform->scale = 1;
    Geometry *g = oneoff_resource(Geometry, b->geometry);
    *g = geometry;
    g->mesh_data = malloc(sizeof(MeshData));
    mem_check(g->mesh_data);
    memcpy(g->mesh_data, &mesh, sizeof(MeshData)); //save the geometry in application memory.
    //b->material = Material_create("Materials/red");
#if 0
    b->material = Material_create("Materials/textured_phong_shadows_phong_tessellation");
#else
    b->material = Material_create("Materials/textured_phong_shadows");
#endif
    if (frand() > 0.5) material_set_texture_path(resource_data(Material, b->material), "diffuse_map", "Textures/minecraft/stone_bricks");
    else material_set_texture_path(resource_data(Material, b->material), "diffuse_map", "Textures/minecraft/dirt");

    RigidBody *rb = add_aspect(e, RigidBody);
    RigidBody_init_polytope(rb, polyhedron_points(poly), polyhedron_num_points(&poly), 1);
    float s = 30;
    // rb->linear_momentum = new_vec3(frand()*s-s/2,frand()*s-s/2,frand()*s-s/2);
    // float r = 1000;
    // rb->angular_momentum = new_vec3(frand()*r-r/2,frand()*r-r/2,frand()*r-r/2);

    hull = poly;
    return rb;
}

void create_random_object(int kind)
{
    float d = 300;
    create_object(kind, new_vec3(frand()*d-d/2, frand()*d-d/2, frand()*d-d/2), new_vec3(0,0,0));
}

bool wireframe = false;
extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_V) for (int i = 0; i < 20; i++) create_random_object(ROCK);
        if (key == GLFW_KEY_P) create_random_object(ROCK);
        if (key == GLFW_KEY_O) create_random_object(ROD);
        if (key == GLFW_KEY_I) create_random_object(PLATE);
        if (key == GLFW_KEY_C) create_random_object(ICOSAHEDRON);
        if (key == GLFW_KEY_V) viewing = !viewing;
        if (key == GLFW_KEY_M) {
            wireframe = !wireframe;
            for_aspect(Body, body)
                body->visible = !wireframe;
            end_for_aspect()
        }
    }
}
    
extern void mouse_button_event(int button, int action, int mods)
{
}
extern void cursor_move_event(double x, double y)
{
}

void turn_on(void)
{
}
void turn_off(void)
{
    for_aspect(Body, body)
        body->visible = false;
    end_for_aspect() } 

Polyhedron bunny_hull;
mat4x4 bunny_matrix;
extern void init_program(void)
{
    test_directional_light_controlled();
    // open_scene(g_scenes, "block_on_floor");
    create_key_camera_man(0,0,0,  0,0,0);
    //create_object(ROCK);

    // RigidBody *test_rod = create_object(ROD, new_vec3(0,-150,0), new_vec3(0,0.3,0));
    // test_rod->angular_momentum = new_vec3(0,0,600);
    RigidBody *test_rod = create_object(ROD, new_vec3(0,-150,0), new_vec3(0,0.3,M_PI/4));
    test_rod->linear_momentum = new_vec3(0,-10,0);

    EntityID e = new_entity(4);
    Transform_set(add_aspect(e, Transform), 0,-50000-500,0,0,0,0);
    Body *body = add_aspect(e, Body);
    get_aspect_type(e, Transform)->scale = 100000;
    body->visible = true;
    body->geometry = new_resource_handle(Geometry, "Models/block -a");
    body->material = Material_create("Materials/textured_phong_shadows");
    body->is_ground = true;
    material_set_texture_path(resource_data(Material, body->material), "diffuse_map", "Textures/marble_tile");
    Geometry *block_geometry = resource_data(Geometry, body->geometry);
    RigidBody_init_polytope(add_aspect(e, RigidBody), block_geometry->mesh_data->attribute_data[Position], block_geometry->num_vertices, 0);
#if 0
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
#endif
}
extern void loop_program(void)
{
    // draw_polyhedron(&hull, NULL);
    // draw_polyhedron_winding_order(&hull, "k", 10, NULL);
    draw_polyhedron(&bunny_hull, &bunny_matrix);
    draw_polyhedron_winding_order(&bunny_hull, "k", 10, &bunny_matrix);
    for_aspect(Body, body)
        if (wireframe) MeshData_draw_wireframe(resource_data(Geometry, body->geometry)->mesh_data, Transform_matrix(other_aspect(body, Transform)), new_vec4(0.5,0,0,1), 3);
    end_for_aspect()

    vec3 origin = vec3_zero();
    paint_line_cv(Canvas3D, origin, new_vec3(10,10,10), "p", 10);
    paint_line_cv(Canvas3D, origin, new_vec3(-10,10,10), "p", 10);
#if 1 //gravity
    if (!TEST_SWITCH) {
        for_aspect(RigidBody, rb)
             rb->linear_momentum.vals[1] -= rb->mass * dt * 500;
        end_for_aspect()
    }
#endif
}
extern void close_program(void)
{
}
