/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"
Polyhedron poly;

void create(void)
{
    poly = random_convex_polyhedron(100, 100);
    PolyhedronPoint *p = poly.points.first;
    float o = 70;
    vec3 shift = new_vec3(o*frand()-o/2,o*frand()-o/2,o*frand()-o/2);
    while (p != NULL) {
        p->position = vec3_add(p->position, shift);
        p = p->next;
    }
}

static void show_simplex(int n, vec3 simplex[], char *color_str)
{
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < i; j++) {
            paint_line_cv(Canvas3D, simplex[i], simplex[j], color_str, 20);
        }
    }
}

int N = 0;
void contains_origin(Polyhedron poly)
{
    int counter = 0;
#define check() if (N == counter++)
    vec3 simplex[4];
    int n = 2;
    simplex[0] = polyhedron_extreme_point(poly, new_vec3(1,1,1));
    simplex[1] = polyhedron_extreme_point(poly, vec3_neg(simplex[0]));
    vec3 origin = vec3_zero();

    while (1) {
        printf("num points: %d\n", n);
        vec3 c = closest_point_on_simplex(n, simplex, origin);
        vec3 dir = vec3_neg(c);
        if (n == 4 && point_in_tetrahedron(simplex[0],simplex[1],simplex[2],simplex[3], origin)) {
            show_simplex(n, simplex, "g");
            paint_points_c(Canvas3D, &origin, 1, "tg", 50);

            // Find the closest point on the boundary.
            // The negative of this will be the minimal separating vector the polyhedron has from the origin.
            // -------------------------------------------------------------------------------
            // Brute force it for comparison.
            PolyhedronTriangle *tri = poly.triangles.first;
            vec3 brute_p = closest_point_on_triangle_to_point(tri->points[0]->position,tri->points[1]->position,tri->points[2]->position, origin);
            while ((tri = tri->next) != NULL) {
                vec3 new_p = closest_point_on_triangle_to_point(tri->points[0]->position,tri->points[1]->position,tri->points[2]->position, origin);
                if (vec3_dot(new_p, new_p) < vec3_dot(brute_p, brute_p)) brute_p = new_p;
            }
	    paint_points_c(Canvas3D, &brute_p, 1, "g", 30);

            // Perform the expanding polytope algorithm.

            // Instead of using a fancy data-structure, the polytope is maintained by keeping
            // a pool. Entries can be nullified, and entries readded in those empty spaces, but linear iterations still need to
            // check up to *_len features. If the arrays are not long enough, then this fails.
            float points[1024 * 3] = {-1}; //----should be indices. If coord is -1, this is definitely buggy.
            const int points_n = 3;
            int points_len = 0;
            int16_t triangles[1024 * 3] = {-1};
            const int triangles_n = 3;
            int triangles_len = 0;
            int16_t edges[1024 * 2] = {-1};
            const int edges_n = 2;
            int edges_len = 0;

            // For efficiency, features are added by this simple macro'd routine. e.g.,
            //     int new_tri_index;
            //     new_feature_index(triangles, new_tri_index);
            // will get the index of an available triangle slot.
            static const char *EPA_error_string = ERROR_ALERT "Expanding polytope algorithm: Fixed-length feature lists failed to be sufficient.\n";
            #define new_feature_index(TYPE,INDEX)\
            {\
                bool found = false;\
                for (int i = 0; i < TYPE ## _len; i++) {\
                    if (TYPE [TYPE ## _n * i] == -1) {\
                        ( INDEX ) = i;\
                        found = true;\
                        break;\
                    }\
                }\
                if (!found) {\
                    if (TYPE ## _len == 1024) {\
                        fprintf(stderr, EPA_error_string);\
                        exit(EXIT_FAILURE);\
                    }\
                    ( INDEX ) = TYPE ## _len ++;\
                }\
            }
            #define add_point(VEC) {\
                int index;\
                new_feature_index(points, index);\
                points[points_n * index] = ( VEC ).vals[0];\
                points[points_n * index + 1] = ( VEC ).vals[1];\
                points[points_n * index + 2] = ( VEC ).vals[2];\
            }
            #define add_edge(AI,BI) {\
                int index;\
                new_feature_index(edges, index);\
                edges[edges_n * index] = ( AI );\
                edges[edges_n * index + 1] = ( BI );\
            }
            #define add_triangle(AI,BI,CI) {\
                int index;\
                new_feature_index(triangles, index);\
                triangles[triangles_n * index] = ( AI );\
                triangles[triangles_n * index + 1] = ( BI );\
                triangles[triangles_n * index + 2] = ( CI );\
                add_edge(AI,BI);\
                add_edge(BI,CI);\
                add_edge(CI,AI);\
            }
            float v = tetrahedron_6_times_volume(simplex[0],simplex[1],simplex[2],simplex[3]);
            if (v < 0) {
                // If the tetrahedron has negative volume, swap two entries, forcing the winding order to be anti-clockwise from outside.
                vec3 temp = simplex[0];
                simplex[0] = simplex[1];
                simplex[1] = temp;
            }
            //---since it is known that everything is empty, it would be more efficient to just hardcode the initial tetrahedron.
            for (int i = 0; i < 4; i++) {
                add_point(simplex[i]);
            }
            add_triangle(0,1,2);
            add_triangle(1,0,3);
            add_triangle(2,1,3);
            add_triangle(0,2,3);
            // The initial tetrahedron has been set up. Proceed with EPA.
            while (1) {
                // Find the closest triangle to the origin.
                float min_d = -1;
                int closest_triangle_index = -1;
                for (int i = 0; i < triangles_len; i++) {
                    if (triangles[3*i] == -1) continue;
                    vec3 a = points[triangles[3*i]];
                    vec3 b = points[triangles[3*i+1]];
                    vec3 c = points[triangles[3*i+2]];
                    vec3 p = point_to_triangle_plane(a,b,c, origin);
                    float new_d = vec3_dot(p, p);
                    if (min_d == -1 || new_d < min_d) {
                        min_d = new_d;
                        closest_triangle_index = i;
                    }
                }
                vec3 a = triangles[3*closest_triangle_index];
                vec3 b = triangles[3*closest_triangle_index + 1];
                vec3 c = triangles[3*closest_triangle_index + 2];
                vec3 expand_to = vec3_cross(vec3_sub(b, a), vec3_sub(c, a));
                vec3 new_point = polyhedron_extreme_point(poly, expand_to);
            }


            return;
        }
        vec3 new_point = polyhedron_extreme_point(poly, dir);

        bool on_simplex = false;
        for (int i = 0; i < n; i++) {
            if (vec3_equal(simplex[i], new_point)) {
                on_simplex = true;
                break;
            }
        }
        if (n == 4 && !on_simplex) {
            int replace = simplex_extreme_index(n, simplex, c);
            simplex[replace] = new_point;
        } else if (n == 3 && on_simplex) {
            show_simplex(n, simplex, "r");
            paint_points_c(Canvas3D, &origin, 1, "tr", 50);
            vec3 closest_on_poly = closest_point_on_triangle_to_point(simplex[0], simplex[1], simplex[2], origin);
            paint_points_c(Canvas3D, &closest_on_poly, 1, "tb", 50);
            return;
        } else if (n == 2 && on_simplex) {
            show_simplex(n, simplex, "r");
            paint_points_c(Canvas3D, &origin, 1, "tr", 50);
            vec3 closest_on_poly = closest_point_on_line_segment_to_point(simplex[0], simplex[1], origin);
            paint_points_c(Canvas3D, &closest_on_poly, 1, "tb", 50);
        } else if (!on_simplex) {
            simplex[n++] = new_point;
        } else {
            int remove = simplex_extreme_index(n, simplex, c);
            for (int j = remove; j < n - 1; j++) {
                simplex[j] = simplex[j + 1];
            }
            n --;
        }

        check() {
            show_simplex(n, simplex, "k");
            return;
        }
    }
}


extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_R) {
            create();
        }
        if (key == GLFW_KEY_T) N ++;
        if (key == GLFW_KEY_O) N = 0;
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
    create_key_camera_man(0,0,0,  0,0,0);
    create();
}
extern void loop_program(void)
{
    vec3 origin = vec3_zero();
    paint_points_c(Canvas3D, &origin, 1, "p", 25);
    draw_polyhedron(&poly, NULL);
    contains_origin(poly);
}
extern void close_program(void)
{
}
