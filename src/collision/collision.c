/*--------------------------------------------------------------------------------
project_libs:
    + Engine
    + geometry
--------------------------------------------------------------------------------*/
#include "Engine.h"
#include "geometry.h"

vec3 *random_points(float radius, int n)
{
    // So the "random" convex polyhedra have some sort of variety, use Gram-Schmidt to create a semi-random orthonormal basis, and
    // then have r1,r2,r3 be the "random principal axes", that weight the points in each of those directions.
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
    for (int i = 0; i < n; i++) {
        points[i] = vec3_mul(vec3_add(vec3_add(vec3_mul(e1, frand()-0.5), vec3_mul(e2, frand()-0.5)), vec3_mul(e3, frand()-0.5)), radius);
        print_vec3(points[i]);
    }
    return points;
}

vec3 *points;
int num_points;
Polyhedron hull;
bool viewing = false;

void create_object(void)
{
    points = random_points(50, num_points);
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
    //float d = 300;
    //Transform_set(add_aspect(e, Transform), frand()*d-d/2, frand()*d-d/2, frand()*d-d/2, 0,0,0);
    Transform_set(add_aspect(e, Transform), 0,0,0,0,0,0);
    Body *b = add_aspect(e, Body);
    b->visible = true;
    b->scale = 1;
    Geometry *g = oneoff_resource(Geometry, b->geometry);
    *g = geometry;
    // b->material = Material_create("Materials/red");
    b->material = Material_create("Materials/textured_phong_shadows");
    material_set_texture_path(resource_data(Material, b->material), "diffuse_map", "Textures/minecraft/dirt");

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
extern void init_program(void)
{
    test_directional_light_controlled();
    // open_scene(g_scenes, "block_on_floor");
    create_key_camera_man(0,0,0,  0,0,0);
    num_points = 100;
    create_object();
}
extern void loop_program(void)
{
    paint_points_c(Canvas3D, points, num_points, "b", 5);
    if (viewing) {
        draw_polyhedron(&hull);
        draw_polyhedron_winding_order(&hull, "k", 10);
    }
}
extern void close_program(void)
{
}
