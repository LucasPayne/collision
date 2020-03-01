/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

// int test_triangle = 0;

vec3 barycentric_triangle(vec3 a, vec3 b, vec3 c, float wa, float wb, float wc)
{
    //wa+wb+wc = 1
    return vec3_add(vec3_mul(a, wa), vec3_add(vec3_mul(b, wb), vec3_mul(c, wc)));
}
vec3 triangle_centroid(vec3 a, vec3 b, vec3 c)
{
    return barycentric_triangle(a,b,c, 1.0/3.0, 1.0/3.0, 1.0/3.0);
}
bool barycentric_triangle_convex(float wa, float wb, float wc)
{
    // tests whether the weights are a convex combination of the triangle points.
    return 0 <= wa && wa <= 1 && 0 <= wb && wb <= 1 && 0 <= wc && wc <= 1;
}
bool ray_triangle_intersection(vec3 origin, vec3 direction, vec3 a, vec3 b, vec3 c, vec3 *intersection)
{
    float wa = vec3_dot(direction, vec3_cross(vec3_sub(b, origin), vec3_sub(c, origin)));
    float wb = vec3_dot(direction, vec3_cross(vec3_sub(c, origin), vec3_sub(a, origin)));
    float wc = vec3_dot(direction, vec3_cross(vec3_sub(a, origin), vec3_sub(b, origin)));
    float winv = 1.0 / (wa + wb + wc);
    wa *= winv;
    wb *= winv;
    wc *= winv;
    if (barycentric_triangle_convex(wa,wb,wc)) {
        vec3 p = barycentric_triangle(a,b,c, wa,wb,wc);
        if (vec3_dot(vec3_sub(p, origin), direction) >= 0) {
            *intersection = p;
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------
struct node;
struct node {
    bool done; // this flag is here for use when traversing this graph.
    uint32_t adjacent_triangles[3]; // ab, bc, ca
};
void mesh_draw_wrong_winding_orders(Body *body, MeshData *mesh_data, int recursion_depth)
{
    void propogate_triangle_orientation(mat4x4 *draw_matrix, vec3 *positions, uint32_t *triangles, struct node *triangle_graph, uint32_t triangle, bool is_outer, int recursion_depth);

    mat4x4 body_matrix = Body_matrix(body); // This matrix is used for rendering things on the mesh, for visualization.

    int starting_triangle = 78; //-----this is set specifically for the bunny model, to test the intersection methods, because this triangle is on the ear facing
                                // down toward the body.
    
    if (mesh_data->num_triangles < 1) return;
    // Select the triangle to be the start of the winding-test propogation.
    int first_a = mesh_data->triangles[3*starting_triangle + 0];
    int first_b = mesh_data->triangles[3*starting_triangle + 1];
    int first_c = mesh_data->triangles[3*starting_triangle + 2];
    vec3 *positions = (vec3 *) (mesh_data->attribute_data[Position]);

    // Determine the correctness of the winding order geometrically by casting a ray from the surface
    // normal outward. If there are an even number of triangle intersections, then this first triangle has a correct winding order.
    // This is subject to geometric robustness issues which aren't addressed here.
    vec3 centroid = triangle_centroid(positions[first_a], positions[first_b], positions[first_c]);
    vec3 normal = vec3_cross(vec3_sub(positions[first_b], positions[first_a]), vec3_sub(positions[first_c], positions[first_a]));
    int num_intersections = 0;
    vec3 last_intersection = centroid; // for line visualization.
    for (int i = 0; i < mesh_data->num_triangles; i++) {
        if (i == starting_triangle) continue;
        vec3 a = positions[mesh_data->triangles[3*i + 0]];
        vec3 b = positions[mesh_data->triangles[3*i + 1]];
        vec3 c = positions[mesh_data->triangles[3*i + 2]];
        vec3 in;
        if (ray_triangle_intersection(centroid,normal, a,b,c, &in)) {
            paint_line_cv(Canvas3D, mat4x4_vec3(&body_matrix, last_intersection), mat4x4_vec3(&body_matrix, in), num_intersections % 2 == 0 ? "b" : "g", 50)
            last_intersection = in;
            num_intersections ++;
        }
    }
    paint_line_cv(Canvas3D, mat4x4_vec3(&body_matrix, last_intersection), mat4x4_vec3(&body_matrix, vec3_add(last_intersection, vec3_mul(normal, 1000))), num_intersections % 2 == 0 ? "b" : "g", 50)

    // If this is a correctly formed closed connected mesh with an interior, then every triangle will have three adjacent triangles, and this algorithm should work.
    // Given a representation of the mesh for which from a triangle, the three adjacent triangles can be obtained, then
    // from the first (now correctly oriented) triangle, the correct winding order can be propogated across the whole mesh
    // until every triangle has been marked as reoriented.
    
    // What is wanted is the graph allowing traversal of the mesh from faces through to adjacent faces.
    // Instead of doing a brute-force search for triangles sharing edges for each triangle, construct an intermediary graph first,
    // which connects points to triangles incident to them.
    #define max_num_adjacent_triangles 64
         // If this becomes a problem, either change this number or allow the array to grow for rare cases.
    struct point_triangles {
        int num_triangles;
        uint32_t triangles[max_num_adjacent_triangles];
    };
    struct point_triangles *point_triangle_arrays = (struct point_triangles *) calloc(mesh_data->num_vertices, sizeof(struct point_triangles));
    mem_check(point_triangle_arrays);
    // Build up adjacent-triangle arrays for each vertex.
    for (int i = 0; i < mesh_data->num_triangles; i++) {
        // For each vertex in the triangle, add the triangle index to that vertex's adjacent-triangle array.
        for (int j = 0; j < 3; j++) {
            uint32_t vertex_index = mesh_data->triangles[3*i + j];
            if (point_triangle_arrays[vertex_index].num_triangles >= max_num_adjacent_triangles) {
                fprintf(stderr, ERROR_ALERT "too many triangles incident to one vertex.");
                exit(EXIT_FAILURE);
            }
            point_triangle_arrays[vertex_index].triangles[point_triangle_arrays[vertex_index].num_triangles] = i; // Add this triangle index to this vertex's triangle array.
            point_triangle_arrays[vertex_index].num_triangles ++;
        }
    }
    // Next, using this intermediary graph, go over each triangle, and go over each triangle edge, taking the two points
    // and brute-force searching their incident triangles for a triangle in common which is not this triangle, which should be
    // the adjacent triangle at this edge.
    struct node *triangle_graph = (struct node *) malloc(mesh_data->num_triangles * sizeof(struct node));
    mem_check(triangle_graph);
    for (int i = 0; i < mesh_data->num_triangles; i++) {
        vec3 a = positions[mesh_data->triangles[3*i + 0]];
        vec3 b = positions[mesh_data->triangles[3*i + 1]];
        vec3 c = positions[mesh_data->triangles[3*i + 2]];
        vec3 centroid = triangle_centroid(a, b, c);  // calculate these for visualization of the graph.

        for (int j = 0; j < 3; j++) {
            int alpha = mesh_data->triangles[3*i + j];
            int beta = mesh_data->triangles[3*i + ((j + 1) % 3)];
            triangle_graph[i].adjacent_triangles[j] = -1; // if somehow this edge doesn't have an adjacent triangle, use -1.
            for (int k = 0; k < point_triangle_arrays[alpha].num_triangles; k++) {
                for (int h = 0; h < point_triangle_arrays[beta].num_triangles; h++) {
                    uint32_t tri_alpha = point_triangle_arrays[alpha].triangles[k];
                    uint32_t tri_beta = point_triangle_arrays[beta].triangles[h];
                    if ((tri_alpha == tri_beta) && (tri_alpha != i)) { // this second case is to exclude the triangle the edge is taken from.
                        triangle_graph[i].adjacent_triangles[j] = tri_alpha;
                    }
                }
            }
            if (triangle_graph[i].adjacent_triangles[j] != -1) {
                vec3 ta = positions[mesh_data->triangles[3*triangle_graph[i].adjacent_triangles[j] + 0]];
                vec3 tb = positions[mesh_data->triangles[3*triangle_graph[i].adjacent_triangles[j] + 1]];
                vec3 tc = positions[mesh_data->triangles[3*triangle_graph[i].adjacent_triangles[j] + 2]];
                vec3 tcentroid = triangle_centroid(ta, tb, tc);
                paint_line_cv(Canvas3D, mat4x4_vec3(&body_matrix, centroid), mat4x4_vec3(&body_matrix, tcentroid), "tr", 2.0);
            }
        }
    }
    printf("num_intersections: %d\n", num_intersections);
    propogate_triangle_orientation(&body_matrix, positions, mesh_data->triangles, triangle_graph, starting_triangle, num_intersections % 2 == 0, recursion_depth);
}
void propogate_triangle_orientation(mat4x4 *draw_matrix, vec3 *positions, uint32_t *triangles, struct node *triangle_graph, uint32_t triangle, bool is_outer, int recursion_depth)
{
    triangle_graph[triangle].done = true;
    if (!is_outer) {
        // printf("Found an inner-pointing triangle, %u.\n", triangle);
        vec3 pos_a = positions[triangles[3*triangle + 0]];
        vec3 pos_b = positions[triangles[3*triangle + 1]];
        vec3 pos_c = positions[triangles[3*triangle + 2]];
        paint_triangle_cv(Canvas3D, mat4x4_vec3(draw_matrix, pos_a), mat4x4_vec3(draw_matrix, pos_b), mat4x4_vec3(draw_matrix, pos_c), "tk");
    }
    if (recursion_depth == 0) return;
    for (int i = 0; i < 3; i++) {
        uint32_t adj = triangle_graph[triangle].adjacent_triangles[i];
        if (adj == -1) continue;
        if (triangle_graph[adj].done) continue;
        bool adj_is_outer;
        for (int j = 0; j < 3; j++) {
            if (triangle_graph[adj].adjacent_triangles[j] == triangle) {
                uint32_t a = triangles[3*triangle + i];
                //uint32_t b = triangles[3*triangle + (i+1)%3];
                uint32_t adj_a = triangles[3*adj + j];
                //uint32_t adj_b = triangles[3*adj + (j+1)%3];
                // If all is well and good, then these two triangles are adjacent at a and b and just a and adj_a can be used to compare their winding order.
                adj_is_outer = a == adj_a ? !is_outer : is_outer;
                break;
            }
        }
        propogate_triangle_orientation(draw_matrix, positions, triangles, triangle_graph, adj, adj_is_outer, recursion_depth - 1);
    }
}

extern void input_event(int key, int action, int mods)
{
}
extern void mouse_button_event(int button, int action, int mods)
{
}
extern void cursor_move_event(double x, double y)
{
}

int propogation = 0;
void test_input(Input *input, int key, int action, int mods)
{
    Body *body = other_aspect(input, Body);
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_P) {
            //mesh_draw_wrong_winding_orders(body, resource_data(Geometry, body->geometry)->mesh_data, 0);
        }
        if (key == GLFW_KEY_V) {
            body->visible = !body->visible;
        }
        if (key == GLFW_KEY_Y) {
            propogation = 0;
        }
        // if (key == GLFW_KEY_F) {
        //     test_triangle ++;
        //     printf("%d\n", test_triangle);
        // }
    }
}
void test_logic(Logic *logic)
{
    Body *body = other_aspect(logic, Body);
    mesh_draw_wrong_winding_orders(body, resource_data(Geometry, body->geometry)->mesh_data, propogation / 10);
    propogation ++;
}

extern void init_program(void)
{
    open_scene(g_scenes, "floor");
    test_directional_light_controlled();
    create_key_camera_man(0,0,0,  0,0,0);

    EntityID e = new_entity(4);
    float apart = 400;
    Body *body = add_aspect(e, Body);
    body->visible = true;
#define opt 1
#if opt == 1
    Transform_set(add_aspect(e, Transform), 0, -49, 0, 0,0,0);
    body->scale = 250;
    body->geometry = new_resource_handle(Geometry, "Models/stanford_bunny_broken -a");
    body->material = Material_create("Materials/textured_phong_shadows_normal_mapped");
    material_set_texture_path(resource_data(Material, body->material), "normal_map", "Textures/brick_wall_normal");
#elif opt == 2
    Transform_set(add_aspect(e, Transform), 0, -20, 0, -M_PI/2,0,0);
    body->scale = 0.04;
    body->geometry = new_resource_handle(Geometry, "Models/dolphins_broken -a");
    body->material = Material_create("Materials/textured_phong_shadows");
#elif opt == 3
    Transform_set(add_aspect(e, Transform), 0, -46, 0, 0,0,0);
    body->scale = 150;
    body->geometry = new_resource_handle(Geometry, "Models/apple -a");
    body->material = Material_create("Materials/textured_phong_shadows");
#endif
    material_set_texture_path(resource_data(Material, body->material), "diffuse_map", "Textures/minecraft/stone_bricks");
    Input_init(add_aspect(e, Input), INPUT_KEY, test_input, true);
    Logic_init(add_aspect(e, Logic), test_logic);

}
extern void loop_program(void)
{
    
}
extern void close_program(void)
{
}
