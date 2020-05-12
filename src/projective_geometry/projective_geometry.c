/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

#include "patches.c"


PlayerController *g_player;

// Just put it really far away.
vec3 point_at_infinity(vec3 direction)
{
    return vec3_mul(direction, 1500);
}


const int mapped_indices[4] = {
    0,1,2,7
};
typedef struct FrustumDemo_s {
    vec3 box_points[8];
    vec3 box_fifth_point;
    vec3 frustum_points[8];
    vec3 frustum_fifth_point;

    vec3 mapped_points[4]; // 0,1,2,7 indices.
    vec3 fifth_mapped_point; // The extra point which determines the homography.
    vec3 lerp_to_points[4];
    vec3 lerp_to_fifth_point;

    mat4x4 computed_homography;
    mat4x4 inverse_computed_homography;

    // vec3 mapped_points[8]; // near quad, far quad

    bool lerping;
    int lerp_index;
    float lerp_distances[4];
    float lerp_distance_fifth_point;

    vec3 last_mapped_point_transform_positions[5]; // to check if they have moved.
    Transform *mapped_point_transforms[5];

    bool show_triangles;
    vec3 last_position;
} FrustumDemo;
FrustumDemo *g_focused_frustum_demo = NULL;

void FrustumDemo_key_listener(Logic *g, int key, int action, int mods)
{
    FrustumDemo *fd = g->data;
    Transform *t = Transform_get_a(g);
    for (int i = 0; i < 3; i++) {
        if (Transform_position(t).vals[i] != fd->last_position.vals[i]) {
            g_focused_frustum_demo = fd;
        }
    }
    fd->last_position = Transform_position(t);

    if (fd != g_focused_frustum_demo) return;

    if (action == GLFW_PRESS && key == GLFW_KEY_B) {
        fd->lerping = true;
        fd->lerp_index = 0;
        for (int i = 0; i < 4; i++) {
            fd->lerp_to_points[i] = fd->box_points[mapped_indices[i]];
            fd->lerp_distances[i] = vec3_length(vec3_sub(fd->mapped_points[i], fd->lerp_to_points[i]));
        }
        fd->lerp_to_fifth_point = fd->box_fifth_point;
        fd->lerp_distance_fifth_point = vec3_length(vec3_sub(fd->fifth_mapped_point, fd->lerp_to_fifth_point));
    }
    if (action == GLFW_PRESS && key == GLFW_KEY_F) {
        fd->lerping = true;
        fd->lerp_index = 0;
        for (int i = 0; i < 4; i++) {
            fd->lerp_to_points[i] = fd->frustum_points[mapped_indices[i]];
            fd->lerp_distances[i] = vec3_length(vec3_sub(fd->mapped_points[i], fd->lerp_to_points[i]));
        }
        fd->lerp_to_fifth_point = fd->frustum_fifth_point;
        fd->lerp_distance_fifth_point = vec3_length(vec3_sub(fd->fifth_mapped_point, fd->lerp_to_fifth_point));
    }
}

mat4x4 compute_homography(vec3 points[/* 5 */], vec3 images[/* 5 */])
{
    vec4 q = vec3_to_vec4(points[4]);
    vec4 qp = vec3_to_vec4(images[4]);
    mat4x4 A, B;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            A.vals[4*i + j] = images[i].vals[j];
            B.vals[4*i + j] = points[i].vals[j];
        }
        A.vals[4*i + 3] = 1;
        B.vals[4*i + 3] = 1;
    }
    vec4 beta = matrix_vec4(mat4x4_inverse(B), q);
    vec4 alpha = matrix_vec4(mat4x4_inverse(A), qp);
    mat4x4 A_weighted;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            A_weighted.vals[4*i + j] = (alpha.vals[i]/beta.vals[i]) * A.vals[4*i + j];
        }
    }
    mat4x4 homography = mat4x4_multiply(A_weighted, mat4x4_inverse(B));
    return homography;

    //mat4x4 M = mat4x4_multiply(A, mat4x4_inverse(B));
    //vec4 v = matrix_vec4(mat4x4_inverse(M), qp);
    //vec4 lambdas;
    //for (int i = 0; i < 4; i++) lambdas.vals[i] = v.vals[i] / q.vals[i];
    //mat4x4 homography;
    //for (int i = 0; i < 4; i++) {
    //    for (int j = 0; j < 4; j++) {
    //        homography.vals[4*i + j] = lambdas.vals[j] * M.vals[4*i + j];
    //    }
    //}
    //return homography;


    // vec4 beta = mat4x4_solve(B, q);
    // vec4 alpha = mat4x4_solve(A, qp);
    // mat4x4 beta_diag;
    // for (int i = 0; i < 4; i++) beta_diag.vals[4*i + i] = beta.vals[i];
    // vec4 lambdas = mat4x4_solve(beta_diag, alpha);
    // // vec4 lambdas = new_vec4(X(alpha) / X(beta), Y(alpha) / Y(beta), Z(alpha) / Z(beta), W(alpha) / W(beta));
    // mat4x4 lambda_diag;
    // for (int i = 0; i < 4; i++) lambda_diag.vals[4*i + i] = lambdas.vals[i];
    // mat4x4 homography = mat4x4_multiply3(A, lambda_diag, mat4x4_inverse(B));
    // for (int i = 0; i < 16; i++) {
    //     homography.vals[i] /= homography.vals[15];
    // }
    // return homography;
}

void FrustumDemo_update(Logic *g)
{
    FrustumDemo *fd = g->data;
    Transform *t = Transform_get_a(g);
    mat4x4 matrix = Transform_matrix(t);
    painting_matrix(matrix);

    // if (fd->lerping) {
    //     for (int i = 0; i < 
    // }


    // if (!fd->lerping) {
    //     for (int i = 0; i < 5; i++) {
    //         Transform *tp = fd->mapped_point_transforms[i];
    //         vec3 p = vec3_sub(Transform_position(tp), Transform_position(t));
    //         if (fd->last_mapped_point_transform_positions[i] != 
    //     }
    // }

    float lerp_seconds = 0.6;
    if (fd->lerping) {
        int index = fd->lerp_index;
        if (index >= 1) {
            index --;
            vec3 m = fd->mapped_points[index];
	    vec3 t = fd->lerp_to_points[index];
	    float d = vec3_square_length(vec3_sub(t, m));
	    if (d < (fd->lerp_distances[index] / 50)*(fd->lerp_distances[index] / 50)) {
	        fd->mapped_points[index] = t;
                if (index == 4) {
                    fd->lerping = false;
                    fd->lerp_index = 0;
                } else {
                    fd->lerp_index ++;
                }
	    } else {
	        fd->mapped_points[index] = vec3_add(m, vec3_mul(vec3_normalize(vec3_sub(t, m)), fd->lerp_distances[index]*dt / lerp_seconds));
	    }
        } else {
            vec3 m = fd->fifth_mapped_point;
            vec3 t = fd->lerp_to_fifth_point;
            float d = vec3_square_length(vec3_sub(t, m));
            if (d < (fd->lerp_distance_fifth_point / 50)*(fd->lerp_distance_fifth_point / 50)) {
                fd->fifth_mapped_point = t;
                fd->lerp_index ++;
            } else {
                fd->fifth_mapped_point = vec3_add(m, vec3_mul(vec3_normalize(vec3_sub(t, m)), 0.5*fd->lerp_distance_fifth_point*dt / lerp_seconds));
            }
        }
    }

    paint_wireframe_box_v(Canvas3D, fd->box_points, new_vec4(0.5,0.5,0.5,0.8), 0.7);
    paint_wireframe_box_v(Canvas3D, fd->frustum_points, new_vec4(0.4,0.4,0.4,0.8), 0.7);

    // for (int i = 0; i < 4; i++) {
    //     paint_line_v(Canvas3D, fd->box_points[mapped_indices[i]], fd->mapped_points[i], new_vec4(0.9,0,0.7,0.9), 0.8);
    //     paint_points(Canvas3D, &fd->mapped_points[i], 1, 0.8,0.2,0.8,1, 18);
    // }
    paint_line_v(Canvas3D, fd->box_fifth_point, fd->fifth_mapped_point, new_vec4(0.9,0,0.7,0.9), 0.8);
    paint_points(Canvas3D, &fd->fifth_mapped_point, 1, 0.9,0.1,0.9,1, 20);

    float radius = 200;
    vec3 xz_square[4];
    get_regular_polygon(xz_square, 4, vec3_zero(), new_vec3(0,1,0), new_vec3(1,0,0), radius);
    vec3 xy_square[4];
    get_regular_polygon(xy_square, 4, vec3_zero(), new_vec3(0,0,1), new_vec3(1,0,0), radius);
    vec3 yz_square[4];
    get_regular_polygon(yz_square, 4, vec3_zero(), new_vec3(1,0,0), new_vec3(0,1,0), radius);

    vec4 color1 = new_vec4(0,0.3,0.9,0.3);
    vec4 color2 = new_vec4(0.3,0.9,0.3,0.5);
    vec4 color3 = new_vec4(0.9,0,0.3,0.3);
    int tess = 2;
    // paint_grid_vv(Canvas3D, xz_square, color1, tess, tess, 0.8);
    // paint_grid_vv(Canvas3D, xy_square, color2, tess, tess, 0.8);
    // paint_grid_vv(Canvas3D, yz_square, color3, tess, tess, 0.8);
    for (int i = 0; i < 4; i++) {
        // paint_line_v(Canvas3D, xz_square[i], xz_square[(i+1)%4], color1, 2);
        // paint_line_v(Canvas3D, xy_square[i], xy_square[(i+1)%4], color2, 2);
        // paint_line_v(Canvas3D, yz_square[i], yz_square[(i+1)%4], color3, 2);
    }
    paint_line_v(Canvas3D, new_vec3(-radius, 0,0), new_vec3(radius,0,0), new_vec4(0,0,0,0.6), 1);
    paint_line_v(Canvas3D, new_vec3(0, -radius, 0), new_vec3(0,radius,0), new_vec4(0,0,0,0.6), 1);
    paint_line_v(Canvas3D, new_vec3(0,0,-radius), new_vec3(0,0,radius), new_vec4(0,0,0,0.6), 1);


    vec3 preimage_points[5];
    vec3 image_points[5];
    for (int i = 0; i < 4; i++) {
        preimage_points[i] = fd->mapped_points[i];
        image_points[i] = fd->box_points[mapped_indices[i]];
    }
    preimage_points[4] = fd->fifth_mapped_point;
    image_points[4] = fd->box_fifth_point;

    paint_points(Canvas3D, preimage_points, 5, 0.8,0,0.9,1, 12);
    paint_points_c(Canvas3D, image_points, 5, "k", 10);

    mat4x4 homography = compute_homography(preimage_points, image_points);
    // Conjugate the homography. This is the one visible to objects being projected.
    fd->computed_homography = mat4x4_multiply3(matrix, homography, mat4x4_inverse(matrix));
    fd->inverse_computed_homography = mat4x4_multiply3(matrix, mat4x4_inverse(homography), mat4x4_inverse(matrix));

    // print_mat4x4(homography);

    mat4x4 homography_inverse = mat4x4_inverse(homography);
    vec3 homography_points[8];
    for (int i = 0; i < 8; i++) {
        //vec4 hp = matrix_vec4(homography_inverse, vec3_to_vec4(fd->box_points[i]));
        vec4 hp = matrix_vec4(homography, vec3_to_vec4(fd->frustum_points[i]));
        homography_points[i] = new_vec3(X(hp)/W(hp), Y(hp)/W(hp), Z(hp)/W(hp));
    }
    paint_wireframe_box_v(Canvas3D, homography_points, new_vec4(0,0,0,0.5), 0.4);

    // q'': q' scaled by intermediate vector components.
    // vec4 qpp;
    // for (int i = 0; i < 4; i++) qpp.vals[i] = qp.vals[i] * v.vals[i];
    // vec4 lambdas = mat4x4_solve(A, qpp);

    // // sigma = diag(lambdas).
    // mat4x4 sigma = {0};
    // for (int i = 0; i < 4; i++) sigma.vals[4*i + i] = lambdas.vals[i];
    // // Construct the homography.
    // mat4x4 homography = mat4x4_multiply3(A, sigma, mat4x4_inverse(B));
    // print_mat4x4(homography);

    if (fd->show_triangles) {
        #define num_tri 3
        vec4 test_triangles[3*num_tri] = {
            {{50, -10,  0,  1}},
            {{60, -15,  20, 1}},
            {{55,  10, -15, 1}},

            {{50, -15,  0,  1}},
            {{55, -15,  40, 1}},
            {{30,  -30, -15, 1}},

            {{70, 20,  0,  1}},
            {{76, 30,  20, 1}},
            {{93, -30, -15, 1}},
        };
        vec4 tri_colors[num_tri] = {
            {{1,0.3,0.3,1}},
            {{0.2,1,0.3,1}},
            {{0.13,0.233,0.93,1}},
        };
        for (int i = 0; i < 3*num_tri; i++) {
            test_triangles[i] = matrix_vec4(homography, test_triangles[i]);
            test_triangles[i] = vec4_mul(test_triangles[i], 1.0 / W(test_triangles[i]));
        }
        for (int TRI = 0; TRI < num_tri; TRI++) {
            paint_triangle_v(Canvas3D, vec4_to_vec3(test_triangles[3*TRI+0]), vec4_to_vec3(test_triangles[3*TRI+1]), vec4_to_vec3(test_triangles[3*TRI+2]), tri_colors[TRI]);
            for (int i = 0; i < 3; i++) {
                paint_line_cv(Canvas3D, vec4_to_vec3(test_triangles[3*TRI+i]), vec4_to_vec3(test_triangles[3*TRI+(i+1)%3]), "k", 1);
            }
        }
    }
}
FrustumDemo *FrustumDemo_create(float x, float y, float z)
{
    EntityID e = new_gameobject(x,y,z, 0,0,0, true);
    Logic *g = add_logic(e, FrustumDemo_update, FrustumDemo);
    Logic_add_input(g, INPUT_KEY, FrustumDemo_key_listener);
    ControlWidget_add(e, 10);
    FrustumDemo *fd = g->data;

    float near = 30;
    float far = 100;
    float near_half_width = 19;
    float near_half_height = 13.6;
    float far_half_width = (far / near) * near_half_width;
    float far_half_height = (far / near) * near_half_height;
    fd->frustum_points[0] = new_vec3(near, -near_half_height, -near_half_width);
    fd->frustum_points[1] = new_vec3(near, near_half_height, -near_half_width);
    fd->frustum_points[2] = new_vec3(near, near_half_height, near_half_width);
    fd->frustum_points[3] = new_vec3(near, -near_half_height, near_half_width);
    fd->frustum_points[4] = new_vec3(far, -far_half_height, -far_half_width);
    fd->frustum_points[5] = new_vec3(far, far_half_height, -far_half_width);
    fd->frustum_points[6] = new_vec3(far, far_half_height, far_half_width);
    fd->frustum_points[7] = new_vec3(far, -far_half_height, far_half_width);
    fd->frustum_fifth_point = vec3_zero();
    float box_size = 25;
    vec3 box_points[8] = {
        {{0, -box_size,-box_size}},
        {{0, box_size,-box_size}},
        {{0, box_size,box_size}},
        {{0, -box_size,box_size}},
        {{box_size, -box_size,-box_size}},
        {{box_size, box_size,-box_size}},
        {{box_size, box_size,box_size}},
        {{box_size, -box_size,box_size}},
    };
    for (int i = 0; i < 8; i++) {
        fd->box_points[i] = box_points[i];
    }
    fd->box_fifth_point = point_at_infinity(new_vec3(-1,0,0));
    for (int i = 0; i < 4; i++) {
        fd->mapped_points[i] = fd->box_points[mapped_indices[i]];
    }
    fd->fifth_mapped_point = fd->box_fifth_point;

#if 0
    for (int i = 0; i < 5; i++) {
        EntityID ecw = new_gameobject(0,0,0, 0,0,0, true);
        Transform *tcw = Transform_get(ecw);
        // tcw->has_parent = true;
        // tcw->parent = Transform_get_a(g);
        ControlWidget *cw = ControlWidget_add(ecw, 10);
        cw->alpha = 0.6;
        if (i < 5) {
            vec3 v = vec3_add(fd->frustum_points[i], Transform_get_position_a(g));
            Transform_set(tcw, UNPACK_VEC3(v), 0,0,0);
        } else {
            vec3 v = vec3_add(fd->frustum_fifth_point, Transform_get_position_a(g));
            Transform_set(tcw, UNPACK_VEC3(v), 0,0,0);
        }
        fd->mapped_point_transforms[i] = tcw;
    }
#endif
    return fd;
}

typedef struct StraightModel_s {
    vec3 plane_point;
    vec3 plane_normal;
    // float plane_height;
    float width;
    float height;
    int grid_tess_x;
    int grid_tess_y;

    EntityID plane_point_controller;

    float plane_size;
    bool enlarge_plane;
} StraightModel;
typedef struct StraightModel_s SM;

vec3 perspective_point(StraightModel *sm, vec3 p);
void draw_projected_segment(StraightModel *sm, vec3 a, vec3 b, vec4 color, vec4 projected_color, float width);
Logic *StraightModel_add(float x, float y, float z, float width, float height, float plane_height);

void StraightModel_mouse_button_listener(Logic *g, MouseButton button, bool click, float x, float y)
{
    StraightModel *sm = g->data;
    vec3 plane[4];
    vec3 right = new_vec3(1,0,0); //---
    get_regular_polygon(plane, 4, sm->plane_point, sm->plane_normal, right, sm->plane_size);
    mat4x4 m = Transform_get_matrix_a(g);
    for (int i = 0; i < 4; i++) plane[i] = mat4x4_vec3(m, plane[i]);

    if (click && button == MouseRight) {
        if (sm->enlarge_plane) {
	    sm->enlarge_plane = false;
        } else {
            vec3 ray_origin, ray_direction;
            Camera_ray(g_main_camera, x, y, &ray_origin, &ray_direction);
            vec3 intersection;
	    if (ray_rectangle_intersection(ray_origin, ray_direction, plane[0], plane[1], plane[2], plane[3], &intersection)) {
                sm->enlarge_plane = true;
            }
        }
    }
}

extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_R) {
            g_player->altitude = 0;
            g_player->azimuth = 0;
        }
    }
}
extern void mouse_button_event(MouseButton button, bool click, float x, float y)
{
}
extern void mouse_position_event(double x, double y)
{
}
extern void mouse_move_event(double x, double y, double dx, double dy)
{
}

vec3 perspective_point(StraightModel *sm, vec3 p)
{
    float t = vec3_dot(sm->plane_point, sm->plane_normal);
    float t_denom = vec3_dot(p, sm->plane_normal);
    bool sign_swap = false;
    if (t_denom < 0) {
        t_denom = -t_denom;
        sign_swap = true;
    }
    const float epsilon = 0.0004;
    t /= t_denom < epsilon ? 0.0004 : t_denom;
    return vec3_mul(p, sign_swap ? -t : t);

    // float z = p.vals[1];
    // if (z < epsilon) return new_vec3(100000,100000,100000);
    // return new_vec3(sm->plane_height * p.vals[0] / z, sm->plane_height, sm->plane_height * p.vals[2] / z);
}

void draw_projected_segment(StraightModel *sm, vec3 a, vec3 b, vec4 color, vec4 projected_color, float width)
{
    paint_line_v(Canvas3D, a, b, color, width);
    vec3 ap = perspective_point(sm, a);
    vec3 bp = perspective_point(sm, b);
    paint_line_v(Canvas3D, a, b, color, width);
    // paint_triangle_v(Canvas3D, new_vec3(0,0,0), ap, bp, projected_color);
    paint_line_v(Canvas3D, ap, bp, projected_color, width);
    // paint_quad_v(Canvas3D, ap, bp, b, a, color);
    // paint_line_v(Canvas3D, vec3_zero(), a, new_vec4(0.01,0.3,0.9,0.3), 0.4);
    // paint_line_v(Canvas3D, vec3_zero(), b, new_vec4(0,0.2,0.13,0.4), 1);
    // paint_line_v(Canvas3D, a, ap, new_vec4(0,0.2,0.13,0.4), 1);
    // paint_line_v(Canvas3D, b, bp, new_vec4(0,0.2,0.13,0.4), 1);
}

void StraightModel_update(Logic *g)
{
    StraightModel *sm = g->data;
    Transform *t = Transform_get_a(g);
    painting_matrix(Transform_get_matrix_a(g));
    // sm->plane_normal = new_vec3(cos(time), sin(time), 0);

    sm->plane_point = Transform_get_position(sm->plane_point_controller);
    
    vec3 base[4] = {
        {{-sm->width/2,0,-sm->height/2}},
        {{sm->width/2,0,-sm->height/2}},
        {{sm->width/2,0,sm->height/2}},
        {{-sm->width/2,0,sm->height/2}},
    };
    vec3 plane[4];
    vec3 right = new_vec3(1,0,0); //---
    int enlarge = sm->enlarge_plane ? 6 : 1;
    get_regular_polygon(plane, 4, sm->plane_point, sm->plane_normal, right, sm->plane_size*enlarge);
    //paint_grid_vv(Canvas3D, plane, new_vec4(0,0.8,0.21,0.63), enlarge*sm->grid_tess_x, enlarge*sm->grid_tess_y, 2);
    if (sm->enlarge_plane) {
        paint_grid_vv(Canvas3D, plane, new_vec4(0,0.8,0.21,0.63), enlarge*sm->grid_tess_x/3, enlarge*sm->grid_tess_y/3, 1.44);
        //paint_quad_vv(Canvas3D, plane, new_vec4(0,0.8,0.21,0.63));
    } else {
        //paint_grid_vv(Canvas3D, plane, new_vec4(0,0.8,0.21,0.63), sm->grid_tess_x, sm->grid_tess_y, 1.44);
        paint_grid_vv(Canvas3D, plane, new_vec4(0,0.8,0.21,0.63), 1, 1, 1.44);
    }
    // for (int i = 0; i < 4; i++) plane[i] = base[i], plane[i].vals[1] += sm->plane_height;
    paint_sphere_v(Canvas3D, new_vec3(0,0,0), 1, new_vec4(0,0,0,1));
    // paint_sphere_v(Canvas3D, plane_origin, 1, new_vec4(0,0.5,0,1));
    // paint_line_v(Canvas3D, origin, plane_origin, new_vec4(0,0,0,1), 0.8);
    //paint_grid_v(Canvas3D, base[0], base[1], base[2], base[3], new_vec4(0,0.3,0.9,0.5), sm->grid_tess_x, sm->grid_tess_y, 1);
    paint_grid_v(Canvas3D, base[0], base[1], base[2], base[3], new_vec4(0,0.3,0.9,0.5), 1, 1, 1);
    // paint_grid_v(Canvas3D, plane[0], plane[1], plane[2], plane[3], new_vec4(0,0.8,0.21,0.63), sm->grid_tess_x, sm->grid_tess_y, 2);
    
    painting_matrix_reset();
}
Logic *StraightModel_add(float x, float y, float z, float width, float height, float plane_height)
{
    EntityID e = new_gameobject(x,y,z, 0,0,0, true);
    ControlWidget_add(e, 10)->alpha = 0.6;
    Logic *g = add_logic(e, StraightModel_update, StraightModel);
    StraightModel *sm = g->data;
    sm->width = width;
    sm->height = height;
    sm->plane_point = new_vec3(0,plane_height,0);
    sm->plane_normal = new_vec3(0,1,0);
    sm->grid_tess_x = 10;
    sm->grid_tess_y = 10;
    sm->plane_size = 120;

    Logic_add_input(g, INPUT_MOUSE_BUTTON, StraightModel_mouse_button_listener);

    EntityID eppc = new_gameobject(0,40,0, 0,0,0, true);
    Transform_get(eppc)->has_parent = true;
    Transform_get(eppc)->parent = Transform_get_a(g);
    ControlWidget *ppc = ControlWidget_add(eppc, 10);
    ppc->alpha = 0.6;
    sm->plane_point_controller = eppc;

    return g;
}

extern void close_program(void)
{
}

typedef struct StraightModelDemo_s {
    StraightModel *sm;
} StraightModelDemo;
void StraightModelDemo_update(Logic *g)
{
    StraightModelDemo *sd = g->data;
    StraightModel *sm = sd->sm;
    Transform *t = Transform_get_a(g);
    mat4x4 matrix = Transform_matrix(t);
    painting_matrix(matrix);

    int n = 80;
    for (int i = 0; i < n; i++) {
        float t = i*2*M_PI/n;
        float tp = (i+1)*2*M_PI/n;
        float transparency = 1;
        float tt = time;
        // float tt = 0;
        vec3 p = new_vec3(30*cos(tt+t), 40, 13*sin(2*t));
        draw_projected_segment(sm, p, new_vec3(30*cos(tt+tp), 40, 13*sin(2*tp)), new_vec4(0,0,0,transparency), new_vec4(1,0,0,transparency), 2.8);
        paint_line_v(Canvas3D, vec3_zero(), p, new_vec4(0.3,0,0,0.5), 0.9);
        vec3 pp = perspective_point(sm, p);
        paint_line_v(Canvas3D, vec3_zero(), pp, new_vec4(0.3,0,0,0.5), 0.9);
    }

    painting_matrix_reset();
}

struct SplineDemo_s;
typedef struct SplineDemo_s {
    int num_points;
    Transform **point_transforms;
    StraightModel *sm;
    Transform *transform;
} SplineDemo;
vec3 SD_pos(SplineDemo *sd, int index)
{
    return Transform_position(sd->point_transforms[index]);
}
SplineDemo *SplineDemo_create(float x, float y, float z, vec3 points[], int num_points, void (*demo_function)(Logic *))
{
    Logic *smg = StraightModel_add(x, y, z, 100, 100, 20);
    StraightModel *sm = smg->data;

    Transform *t = Transform_get_a(smg);
    Logic *g = add_logic(smg->entity_id, demo_function, SplineDemo);
    SplineDemo *sd = g->data;
    sd->transform = t;
    sd->sm = sm;
    sd->num_points = num_points;
    sd->point_transforms = malloc(sizeof(Transform *) * sd->num_points);
    mem_check(sd->point_transforms);
    for (int i = 0; i < sd->num_points; i++) {
        EntityID cwe = new_gameobject(UNPACK_VEC3(points[i]),0,0,0, true);
        Transform *cwt = Transform_get(cwe);
        cwt->has_parent = true;
        cwt->parent = t;
        ControlWidget_add(cwe, 8);
        // float theta = 2*M_PI*i*1.0/sd->num_points;
        // Transform_set_position(cwt, new_vec3(30*cos(theta), 50, 30*sin(theta)));
        //Transform_set_position(cwt, new_vec3(40*frand(),40*frand(),40*frand()));
        sd->point_transforms[i] = cwt;
    }
}
void SplineDemo1_update(Logic *g)
{
    // Polyline.
    SplineDemo *sd = g->data;
    painting_matrix(Transform_get_matrix_a(g));
    for (int i = 0; i < sd->num_points-1; i++) {
        draw_projected_segment(sd->sm, SD_pos(sd, i), SD_pos(sd, i+1), new_vec4(1,0,1,1), new_vec4(0,0,0,1), 4);
    }
    painting_matrix_reset();
}

vec3 evaluate_quadratic_bezier(vec3 points[], float t)
{
    vec3 p01 = vec3_lerp(points[0], points[1], t);
    vec3 p12 = vec3_lerp(points[1], points[2], t);
    return vec3_lerp(p01, p12, t);
}
vec3 evaluate_cubic_bezier(vec3 points[], float t)
{
    vec3 p01 = vec3_lerp(points[0], points[1], t);
    vec3 p12 = vec3_lerp(points[1], points[2], t);
    vec3 p23 = vec3_lerp(points[2], points[3], t);
    vec3 p0112 = vec3_lerp(p01, p12, t);
    vec3 p1223 = vec3_lerp(p12, p23, t);
    return vec3_lerp(p0112, p1223, t);
}
vec3 evaluate_rational_cubic_bezier(vec4 homogeneous_points[], float t)
{
    vec4 p01 = vec4_lerp(homogeneous_points[0], homogeneous_points[1], t);
    vec4 p12 = vec4_lerp(homogeneous_points[1], homogeneous_points[2], t);
    vec4 p23 = vec4_lerp(homogeneous_points[2], homogeneous_points[3], t);
    vec4 p0112 = vec4_lerp(p01, p12, t);
    vec4 p1223 = vec4_lerp(p12, p23, t);
    vec4 p = vec4_lerp(p0112, p1223, t);
    return new_vec3(X(p) / W(p), Y(p) / W(p), Z(p) / W(p));
}


void SplineDemo_draw_control_polyline(SplineDemo *sd)
{
    mat4x4 vp_matrix = Camera_vp_matrix(g_main_camera);

    for (int i = 0; i < sd->num_points-1; i++) {
        vec3 a = SD_pos(sd, i);
        vec3 b = SD_pos(sd, i+1);
        paint_line_cv(Canvas3D, a,b, "tk", 0.8);
        vec3 ap = perspective_point(sd->sm, a);
        vec3 bp = perspective_point(sd->sm, b);
        paint_line_cv(Canvas3D, ap,bp, "tk", 0.8);
    }
    for (int i = 0; i < sd->num_points; i++) {
        vec3 pos = SD_pos(sd, i);
        vec3 p = perspective_point(sd->sm, pos);

        float point_size = 23*(1 - exp(-0.014*Y(pos)));

        // Scroll wheel to increase the "weight" of this point.
        vec3 world_p = mat4x4_vec3(Transform_matrix(sd->transform), p);
        vec2 screen_pos = Camera_world_point_to_screen(g_main_camera, world_p);
        float screen_x = X(screen_pos);
        float screen_y = Y(screen_pos);

        const float click_radius = 0.02;
        const float weight_speed = 5;
        if ((screen_x - mouse_screen_x)*(screen_x - mouse_screen_x) + (screen_y - mouse_screen_y)*(screen_y - mouse_screen_y) < click_radius * click_radius) {
            point_size *= 1.3;
            if (g_y_scroll != 0) {
	        vec3 dir = SD_pos(sd, i);
                sd->point_transforms[i]->x += X(dir) * g_y_scroll * weight_speed * dt;
                sd->point_transforms[i]->y += Y(dir) * g_y_scroll * weight_speed * dt;
                sd->point_transforms[i]->z += Z(dir) * g_y_scroll * weight_speed * dt;
            }
        }

        paint_line_cv(Canvas3D, vec3_zero(), pos, "tk", 0.8);
        paint_line_cv(Canvas3D, vec3_zero(), p, "tk", 0.8);
        
        paint_points(Canvas3D, &p, 1, 0.2,0.2,0.2,1, point_size);
    }
}

void SplineDemo2_update(Logic *g)
{
    // Quadratic spline.
    SplineDemo *sd = g->data;
    painting_matrix(Transform_get_matrix_a(g));

    SplineDemo_draw_control_polyline(sd);

    int tess = 7;
    float inv_tess_plus_one = 1.0 / (tess + 1);

    for (int i = 0; i < sd->num_points-2; i++) {
        vec3 window[3];
        for (int j = 0; j < 3; j++) window[j] = SD_pos(sd, i+j);
        vec3 points[3];
        points[0] = vec3_lerp(window[0], window[1], 0.5);
        points[1] = window[1];
        points[2] = vec3_lerp(window[1], window[2], 0.5);

        
        for (int j = 0; j < tess+1; j++) {
            float t1 = j * inv_tess_plus_one;
            float t2 = (j+1) * inv_tess_plus_one;
            vec3 a = evaluate_quadratic_bezier(points, t1);
            vec3 b = evaluate_quadratic_bezier(points, t2);
            draw_projected_segment(sd->sm, a, b, new_vec4(1,0,1,1), new_vec4(0,0,0,1), 2.3);
        }
    }
    painting_matrix_reset();
}

const mat4x4 cubic_bspline_to_cubic_bernstein = {{
    0, 0, 0, 1.0/6.0,
    1.0/6.0, 1.0/3.0, 2.0/3.0, 2.0/3.0,
    2.0/3.0, 2.0/3.0, 1.0/3.0, 1.0/6.0,
    1.0/6.0, 0,0,0,
}};
void SplineDemo3_update(Logic *g)
{
    // Cubic spline.
    SplineDemo *sd = g->data;

    painting_matrix(Transform_get_matrix_a(g));

    SplineDemo_draw_control_polyline(sd);

    int tess = 7;
    float inv_tess_plus_one = 1.0 / (tess + 1);

    for (int i = 0; i < sd->num_points-3; i++) {
        vec3 window[4];
        for (int j = 0; j < 4; j++) window[j] = SD_pos(sd, i+j);
        vec3 points[4] = {0};
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                points[j] = vec3_add(points[j], vec3_mul(window[k], cubic_bspline_to_cubic_bernstein.vals[4*k + j]));
            }
        }
        for (int j = 0; j < tess+1; j++) {
            float t1 = j * inv_tess_plus_one;
            float t2 = (j+1) * inv_tess_plus_one;
            vec3 a = evaluate_cubic_bezier(points, t1);
            vec3 b = evaluate_cubic_bezier(points, t2);
            draw_projected_segment(sd->sm, a, b, new_vec4(1,0,1,1), new_vec4(0,0,0,1), 2.3);
        }
    }
    painting_matrix_reset();
}

void bunny_update(Logic *logic)
{
    Transform *t = Transform_get_a(logic);
    fill_mat3x3_cmaj(t->rotation_matrix, cos(time), 0, sin(time),
                                         0, 1, 0,
                                         -sin(time), 0, cos(time));

    // Body *body = get_sibling_aspect(logic, Body);
    // MeshData *mesh = resource_data(Geometry, body->geometry)->mesh_data;
    // MeshData_draw_wireframe(mesh, Transform_get_matrix_a(logic), new_vec4(0,0,0,1), 2);
}

typedef struct NURBSDemo_s {
    int n;
    int m;
    Transform **point_transforms;
    Transform *transform;
    float *weights;
} NURBSDemo;

vec3 NURBS_pos(NURBSDemo *nurbs, int i, int j)
{
    return Transform_position(nurbs->point_transforms[nurbs->m * i + j]);
}

void draw_rational_cubic_bezier_patch(vec4 points[], vec4 color, int tess)
{
    float u,v, up,vp;
    float inv_tess_plus_one = 1.0 / (tess + 1);
    for (int i = 0; i < tess; i++) {
        u = i * inv_tess_plus_one;
        up = (i+1) * inv_tess_plus_one;
        for (int j = 0; j < tess; j++) {
            v = j * inv_tess_plus_one;
            vp = (j+1) * inv_tess_plus_one;

            vec3 A = evaluate_rational_cubic_bezier_patch(points, u, v);
            vec3 B = evaluate_rational_cubic_bezier_patch(points, up, v);
            vec3 C = evaluate_rational_cubic_bezier_patch(points, up, vp);
            vec3 D = evaluate_rational_cubic_bezier_patch(points, u, vp);
            print_vec3(A);
            paint_triangle_v(Canvas3D, A, B, C, color);
            paint_triangle_v(Canvas3D, A, C, D, color);
        }
    }
}

void NURBSDemo_update(Logic *g)
{
    NURBSDemo *nurbs = g->data;

    painting_matrix(Transform_get_matrix_a(g));
    for (int i = 0; i < nurbs->n; i++) {
        for (int j = 0; j < nurbs->m; j++) {
            // Scroll wheel to increase the "weight" of this point.
            vec3 p = NURBS_pos(nurbs, i, j);
            vec3 world_p = mat4x4_vec3(Transform_matrix(nurbs->transform), p);
            vec2 screen_pos = Camera_world_point_to_screen(g_main_camera, world_p);
            float screen_x = X(screen_pos);
            float screen_y = Y(screen_pos);
            const float click_radius = 0.02;
            const float weight_speed = 5;
            float point_size = 40*(1 - exp(-0.014*nurbs->weights[nurbs->m * i + j]));
            if ((screen_x - mouse_screen_x)*(screen_x - mouse_screen_x) + (screen_y - mouse_screen_y)*(screen_y - mouse_screen_y) < click_radius * click_radius) {
                point_size *= 1.3;
                if (g_y_scroll != 0) {
                    nurbs->weights[nurbs->m * i + j] += nurbs->weights[nurbs->m * i + j] * g_y_scroll * weight_speed * dt;
                }
            }
            paint_points(Canvas3D, &p, 1, 0.2,0.2,0.2,1, point_size);
        }
    }

    vec4 color = {{0.4,0.4,0.4,0.9}};
    for (int i = 0; i < nurbs->n - 1; i++) {
        for (int j = 0; j < nurbs->m - 1; j++) {
            vec3 a = NURBS_pos(nurbs,i,j);
            vec3 b = NURBS_pos(nurbs,i+1,j);
            vec3 c = NURBS_pos(nurbs,i,j+1);
            paint_line_v(Canvas3D, a, b, color, 1);
            paint_line_v(Canvas3D, a, c, color, 1);
        }
    }
    for (int i = 0; i < nurbs->n - 1; i++) {
        vec3 a = NURBS_pos(nurbs,i,nurbs->m-1);
        vec3 b = NURBS_pos(nurbs,i+1,nurbs->m-1);
        paint_line_v(Canvas3D, a, b, color, 1);
    }
    for (int i = 0; i < nurbs->m - 1; i++) {
        vec3 a = NURBS_pos(nurbs,nurbs->n-1,i);
        vec3 b = NURBS_pos(nurbs,nurbs->n-1,i+1);
        paint_line_v(Canvas3D, a, b, color, 1);
    }

    //vec4 patch_color = new_vec4(0.6,0.6,0.9,1);
    vec4 patch_color = new_vec4(0.3,0.3,0.9,0.7);
    vec4 window[16];
    vec4 bernstein_points[16];
    for (int i = 0; i < nurbs->n - 3; i++) {
        for (int j = 0; j < nurbs->m - 3; j++) {
            for (int ii = 0; ii < 4; ii++) {
                for (int jj = 0; jj < 4; jj++) {
                    vec3 p = NURBS_pos(nurbs, i+ii,j+jj);
                    //print_vec3(p);
                    float w = nurbs->weights[nurbs->m*(i+ii) + j+jj];
                    window[4*ii + jj] = new_vec4(w*X(p), w*Y(p), w*Z(p), w);
                    print_vec4(window[4*ii+jj]);
                }
            }
            for (int ii = 0; ii < 4; ii++) {
                
            }
            for (int jj = 0; jj < 4; jj++) {
            }

	    draw_rational_cubic_bezier_patch(window, patch_color, 5);
        }
    }

    painting_matrix_reset();

}

Logic *NURBSDemo_create(float x, float y, float z, vec3 points[], int n, int m)
{
    EntityID e = new_gameobject(x,y,z, 0,0,0, true);
    
    Transform *t = Transform_get(e);
    Logic *g = add_logic(e, NURBSDemo_update, NURBSDemo);
    NURBSDemo *nurbs = g->data;
    nurbs->transform = t;
    nurbs->n = n;
    nurbs->m = m;
    nurbs->point_transforms = malloc(sizeof(Transform *) * n * m);
    mem_check(nurbs->point_transforms);
    nurbs->weights = malloc(sizeof(float) * n * m);
    mem_check(nurbs->weights);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            EntityID cwe = new_gameobject(UNPACK_VEC3(points[m*i + j]),0,0,0, true);
            Transform *cwt = Transform_get(cwe);
            cwt->has_parent = true;
            cwt->parent = t;
            ControlWidget *cw = ControlWidget_add(cwe, 5);
            cw->alpha = 0.6;
            nurbs->point_transforms[m*i + j] = cwt;
            nurbs->weights[m*i + j] = 50;
        }
    }
    return g;
}


typedef struct SplineDemo3D_s {
    int num_points;
    Transform **point_transforms;
    Transform *transform;
    float *weights;
} SplineDemo3D;

vec3 SD_position_along(SplineDemo3D *sd, float t)
{
    int num_windows = sd->num_points - 3;
    int index = (int) (num_windows * t);
    if (index == num_windows) index --; // The last domain segment includes the end.
    float start = index * 1.0 / num_windows;
    float end = (index + 1) * 1.0 / num_windows;
    float local_t = (t - start) / (end - start);

    vec4 homogeneous_window[4];
    for (int j = 0; j < 4; j++) {
        vec3 p = SD_pos(sd, index+j);
        float w = sd->weights[index+j];
        X(homogeneous_window[j]) = X(p) * w;
        Y(homogeneous_window[j]) = Y(p) * w;
        Z(homogeneous_window[j]) = Z(p) * w;
        W(homogeneous_window[j]) = w;
    }
    vec4 homogeneous_points[4] = {0};
    for (int j = 0; j < 4; j++) {
        for (int k = 0; k < 4; k++) {
            homogeneous_points[j] = vec4_add(homogeneous_points[j],
                                    vec4_mul(homogeneous_window[k], cubic_bspline_to_cubic_bernstein.vals[4*k + j]));
        }
    }
    vec3 a = evaluate_rational_cubic_bezier(homogeneous_points, 1-local_t);
    return a;
}

vec3 SD_pos3D(SplineDemo3D *sd, int index)
{
    return Transform_position(sd->point_transforms[index]);
    // vec3 p = Transform_position(sd->point_transforms[index]);
    // return new_vec4(X(p), Y(p), Z(p), sd->weights[index]);
}
void SplineDemo3D_update(Logic *g)
{
    SplineDemo3D *sd = g->data;
    painting_matrix(Transform_get_matrix_a(g));
    for (int i = 0; i < sd->num_points - 1; i++) {
        paint_line_v(Canvas3D, SD_pos3D(sd, i), SD_pos3D(sd, i+1), new_vec4(0.4,0.4,0.4,0.7), 0.8);
    }
    for (int i = 0; i < sd->num_points; i++) {
        // Scroll wheel to increase the "weight" of this point.
        vec3 p = SD_pos3D(sd, i);
        vec3 world_p = mat4x4_vec3(Transform_matrix(sd->transform), p);
        vec2 screen_pos = Camera_world_point_to_screen(g_main_camera, world_p);
        float screen_x = X(screen_pos);
        float screen_y = Y(screen_pos);
        const float click_radius = 0.02;
        const float weight_speed = 5;
        float point_size = 40*(1 - exp(-0.014*sd->weights[i]));
        if ((screen_x - mouse_screen_x)*(screen_x - mouse_screen_x) + (screen_y - mouse_screen_y)*(screen_y - mouse_screen_y) < click_radius * click_radius) {
            point_size *= 1.3;
            if (g_y_scroll != 0) {
                sd->weights[i] += sd->weights[i] * g_y_scroll * weight_speed * dt;
            }
        }
        paint_points(Canvas3D, &p, 1, 0.2,0.2,0.2,1, point_size);
    }

    int tess = 15;
    float inv_tess_plus_one = 1.0 / (tess + 1);
    for (int i = 0; i < sd->num_points-3; i++) {

        vec4 homogeneous_window[4];
        for (int j = 0; j < 4; j++) {
            vec3 p = SD_pos3D(sd, i+j);
            float w = sd->weights[i+j];
            X(homogeneous_window[j]) = X(p) * w;
            Y(homogeneous_window[j]) = Y(p) * w;
            Z(homogeneous_window[j]) = Z(p) * w;
            W(homogeneous_window[j]) = w;
        }
        vec4 homogeneous_points[4] = {0};
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                homogeneous_points[j] = vec4_add(homogeneous_points[j],
                                        vec4_mul(homogeneous_window[k], cubic_bspline_to_cubic_bernstein.vals[4*k + j]));
            }
        }
        for (int j = 0; j < tess+1; j++) {
            float t1 = j * inv_tess_plus_one;
            float t2 = (j+1) * inv_tess_plus_one;
            vec3 a = evaluate_rational_cubic_bezier(homogeneous_points, t1);
            vec3 b = evaluate_rational_cubic_bezier(homogeneous_points, t2);
            paint_line_v(Canvas3D, a, b, new_vec4(1,0,1,1), 1.9);
        }
    }
    painting_matrix_reset();

}
Logic *SplineDemo3D_create(float x, float y, float z, vec3 points[], int num_points)
{
    EntityID e = new_gameobject(x,y,z, 0,0,0, true);
    
    Transform *t = Transform_get(e);
    Logic *g = add_logic(e, SplineDemo3D_update, SplineDemo3D);
    SplineDemo3D *sd = g->data;
    sd->transform = t;
    sd->num_points = num_points;
    sd->point_transforms = malloc(sizeof(Transform *) * sd->num_points);
    mem_check(sd->point_transforms);
    sd->weights = malloc(sizeof(float) * sd->num_points);
    mem_check(sd->weights);
    for (int i = 0; i < sd->num_points; i++) {
        EntityID cwe = new_gameobject(UNPACK_VEC3(points[i]),0,0,0, true);
        Transform *cwt = Transform_get(cwe);
        cwt->has_parent = true;
        cwt->parent = t;
        ControlWidget *cw = ControlWidget_add(cwe, 5);
        cw->alpha = 0.6;
        sd->point_transforms[i] = cwt;
        sd->weights[i] = 50;
    }
    return g;
}

typedef struct SplineFollower_s {
    SplineDemo3D *sd;    
    Transform *sd_transform;
    float speed;
    float t;
    bool forward;
} SplineFollower; 
void SplineFollower_update(Logic *g)
{
    SplineFollower *f = g->data;
    Transform *t = Transform_get_a(g);

    if (f->forward) {
        f->t += f->speed * dt;
        if (f->t > 1) {
            f->t = 1;
            f->forward = false;
        }
    } else {
        f->t -= f->speed * dt;
        if (f->t < 0) {
            f->t = 0;
            f->forward = true;
        }
    }
    vec3 pos = mat4x4_vec3(Transform_matrix(f->sd_transform), SD_position_along(f->sd, f->t));
    // print_vec3(pos);
    Transform_set_position(t, pos);

    fill_mat3x3_cmaj(t->rotation_matrix, cos(time), 0, sin(time),
                                         0, 1, 0,
                                         -sin(time), 0, cos(time));
}

typedef struct BezierDemo_s {
    //vec3 points[4];
    Transform *point_transforms[4];
    float t;
    float speed;
    bool forward;
    StraightModel *sm;
} BezierDemo;
vec3 BD_point(BezierDemo *bd, int index)
{
    return Transform_position(bd->point_transforms[index]);
}

void BezierDemo_update(Logic *g)
{
    BezierDemo *bd = g->data;
    painting_matrix(Transform_get_matrix_a(g));
    if (bd->forward) {
        bd->t += bd->speed * dt;
        if (bd->t > 1) {
            bd->t = 1;
            bd->forward = false;
        }
    } else {
        bd->t -= bd->speed * dt;
        if (bd->t < 0) {
            bd->t = 0;
            bd->forward = true;
        }
    }
    vec3 perspective_points[4];
    vec3 points[4];
    for (int i = 0; i < 4; i++) {
        points[i] = BD_point(bd, i);
        perspective_points[i] = perspective_point(bd->sm, points[i]);
    }
    vec4 line_color = new_vec4(0.4,0.4,0.4,1);
    for (int i = 0; i < 3; i++) {
        paint_line_v(Canvas3D, points[i], points[i+1], line_color, 1.5);
        paint_line_cv(Canvas3D, perspective_points[i], perspective_points[i+1], "k", 1.5);
    }
    paint_points(Canvas3D, perspective_points, 4, 0,0,0,1, 22);

    float t = bd->t;
    vec3 a = points[0];
    vec3 b = points[1];
    vec3 c = points[2];
    vec3 d = points[3];
    paint_points(Canvas3D, &a, 1, 0,0,0,1, 22);
    paint_points(Canvas3D, &b, 1, 0,0,0,1, 22);
    paint_points(Canvas3D, &c, 1, 0,0,0,1, 22);
    paint_points(Canvas3D, &d, 1, 0,0,0,1, 22);
    vec3 p01 = vec3_lerp(a, b, t);
    vec3 p01p = perspective_point(bd->sm, p01);
    vec3 p12 = vec3_lerp(b, c, t);
    vec3 p12p = perspective_point(bd->sm, p12);
    vec3 p23 = vec3_lerp(c, d, t);
    vec3 p23p = perspective_point(bd->sm, p23);
    paint_points_c(Canvas3D, &p01, 1, "r", 12);
    paint_points_c(Canvas3D, &p12, 1, "r", 12);
    paint_points_c(Canvas3D, &p23, 1, "r", 12);
    paint_points_c(Canvas3D, &p01p, 1, "k", 12);
    paint_points_c(Canvas3D, &p12p, 1, "k", 12);
    paint_points_c(Canvas3D, &p23p, 1, "k", 12);
    paint_line_cv(Canvas3D, p01, p12, "r", 1.5);
    paint_line_cv(Canvas3D, p12, p23, "r", 1.5);
    paint_line_cv(Canvas3D, p01p, p12p, "k", 1.5);
    paint_line_cv(Canvas3D, p12p, p23p, "k", 1.5);
    vec3 p0112 = vec3_lerp(p01, p12, t);
    vec3 p1223 = vec3_lerp(p12, p23, t);
    vec3 p0112p = perspective_point(bd->sm, p0112);
    vec3 p1223p = perspective_point(bd->sm, p1223);
    paint_points_c(Canvas3D, &p0112, 1, "g", 15);
    paint_points_c(Canvas3D, &p1223, 1, "g", 15);
    paint_points_c(Canvas3D, &p0112p, 1, "k", 15);
    paint_points_c(Canvas3D, &p1223p, 1, "k", 15);
    paint_line_cv(Canvas3D, p0112, p1223, "g", 1.5);
    paint_line_cv(Canvas3D, p0112p, p1223p, "k", 1.5);
    vec3 p = vec3_lerp(p0112, p1223, t);
    vec3 pp = perspective_point(bd->sm, p);
    paint_points_c(Canvas3D, &p, 1, "b", 15);
    paint_points_c(Canvas3D, &pp, 1, "k", 15);

    int tess = 12;
    float inv_tess_plus_one = 1.0 / (tess + 1);
    for (int j = 0; j < tess+1; j++) {
        float t1 = j * inv_tess_plus_one;
        float t2 = (j+1) * inv_tess_plus_one;
        vec3 a = evaluate_cubic_bezier(points, t1);
        vec3 b = evaluate_cubic_bezier(points, t2);
        draw_projected_segment(bd->sm, a, b, new_vec4(1,0,1,1), new_vec4(0,0,0,1), 2.3);
    }
    painting_matrix_reset();
}
BezierDemo *BezierDemo_create(float x, float y, float z, vec3 a, vec3 b, vec3 c, vec3 d, float speed)
{
    Logic *smg = StraightModel_add(x, y, z, 100, 100, 20);
    StraightModel *sm = smg->data;
    
    //EntityID e = new_gameobject(x,y,z, 0,0,0, true);
    EntityID e = smg->entity_id;
    Logic *g = add_logic(e, BezierDemo_update, BezierDemo);
    Transform *t = Transform_get_a(g);
    BezierDemo *bd = g->data;
    bd->sm = sm;
    bd->t = 0;
    bd->speed = speed;
    bd->forward = true;

    vec3 points[4];
    points[0] = a;
    points[1] = b;
    points[2] = c;
    points[3] = d;
    for (int i = 0; i < 4; i++) {
        EntityID cwe = new_gameobject(UNPACK_VEC3(points[i]),0,0,0, true);
        Transform *cwt = Transform_get(cwe);
        cwt->has_parent = true;
        cwt->parent = t;
        ControlWidget_add(cwe, 8);
        bd->point_transforms[i] = cwt;
    }

    return bd;
}

extern void init_program(void)
{
    {
    FrustumDemo *fd = FrustumDemo_create(-600,40,0);
    fd->show_triangles = true;
    }

{
    FrustumDemo *fd = FrustumDemo_create(-300,40,0);
{
    EntityID light = new_entity(4);
    Transform *t = entity_add_aspect(light, Transform);
    Transform_set(t, 0,200,0,  M_PI/2+0.3,0,0);
    t->euler_controlled = true;
    DirectionalLight_init(entity_add_aspect(light, DirectionalLight), 1,0.7,0.7,1,  400,400,500);
}
#if 1
    int bunny_square_root = 3;
    for (int i = 0; i < bunny_square_root; i++) {
        for (int j = 0; j < bunny_square_root; j++) {
            EntityID e = new_entity(4);
            float apart = 60;
            Transform *t = add_aspect(e, Transform);
            Transform_set(t, -234 + i*apart,30,(j - bunny_square_root/2.0)*apart, 0,0,0);
            t->scale = 200;
            t->has_freeform_matrix = true;
            t->freeform_matrix = &fd->computed_homography;
            
            Body *body = add_aspect(e, Body);
            //body->scale = 400;
            body->visible = true;
            body->geometry = new_resource_handle(Geometry, "Models/stanford_bunny -a");
#if 0
            body->material = Material_create("Materials/textured_phong");
            material_set_texture_path(resource_data(Material, body->material), "diffuse_map", "Textures/brick_wall");
#endif
            body->material = Material_create("Materials/textured_phong_shadows_normal_mapped");
            material_set_texture_path(resource_data(Material, body->material), "diffuse_map", "Textures/brick_wall");
            material_set_texture_path(resource_data(Material, body->material), "normal_map", "Textures/brick_wall_normal");

            add_empty_logic(e, bunny_update);
        }
    }
#endif
    }


#if 1
#endif
#if 0
    {
    Logic *smg = StraightModel_add(0,0,0, 100, 100, 20);
    StraightModel *sm = smg->data;
    sm->plane_normal = vec3_normalize(new_vec3(0.5,1,0));
    StraightModelDemo *sd = add_logic(smg->entity_id, StraightModelDemo_update, StraightModelDemo)->data;
    sd->sm = sm;
    }
#endif

#if 1
    {
    vec3 points[5];
    get_regular_polygon(points, 5, new_vec3(0,50,0), new_vec3(0,1,0), new_vec3(1,0,0), 40);
    SplineDemo_create(200,0,0, points, 5, SplineDemo1_update);
    }
#endif
    //---for some reason if this is the first entity created, nothing can be seen.
    PlayerController *player = Player_create_default(-600,70,200, 0,0);
    g_player = player;
    player->scrollable_speed = false;
#if 1
    {
    vec3 points[5];
    get_regular_polygon(points, 4, new_vec3(0,50,0), new_vec3(0,1,0), new_vec3(1,0,0), 40);
    //SplineDemo_create(400,0,220, points, 5, SplineDemo2_update);
    SplineDemo_create(400,0,0, points, 5, SplineDemo2_update);
    }
#endif
    {
    #define N 6
    vec3 points[N];
    get_regular_polygon(points, N, new_vec3(0,75,0), new_vec3(0,1,0), new_vec3(1,0,0), 55);
    //SplineDemo_create(400,0,220, points, 5, SplineDemo2_update);
    SplineDemo_create(600,0,0, points, N, SplineDemo3_update);
    }




{
    #define spline_N 7
    vec3 points[spline_N];
    float radius = 100;
    for (int i = 0; i < spline_N; i++) {
        points[i] = vec3_mul(vec3_normalize(new_vec3(frand()-0.5,frand()-0.5,frand()-0.5)), radius);
    }
    Logic *sdg = SplineDemo3D_create(0, 0, 300, points, spline_N);
    SplineDemo *sd = sdg->data;

    for (int i = 0; i < 4; i++) {
        EntityID e = new_entity(4);
        Transform *t = add_aspect(e, Transform);
        Transform_set(t, 0,0,0, 0,0,0);
        t->scale = 120;
        Body *body = add_aspect(e, Body);
        body->visible = true;
        body->geometry = new_resource_handle(Geometry, "Models/stanford_bunny -a");
        body->material = Material_create("Materials/textured_phong_shadows_normal_mapped");
        material_set_texture_path(resource_data(Material, body->material), "diffuse_map", "Textures/brick_wall");
        material_set_texture_path(resource_data(Material, body->material), "normal_map", "Textures/brick_wall_normal");

        Logic *g = add_logic(e, SplineFollower_update, SplineFollower);
        SplineFollower *f = g->data;
        f->sd = sd;
        f->sd_transform = Transform_get_a(sdg);
        f->speed = frand() * 0.2 + 0.05;
        f->forward = frand() > 0.5 ? true : false;
        f->t = frand();
    }
}

#if 0
    // Test 4x4 matrix routines.
    while (1) {
        mat4x4 test;
        vec4 v = new_vec4(1,2,3,4);
        // fill_mat4x4_rmaj(test, 1,2,3,4,
        //                        3,4,5,3,
        //                        -3,2,9,-3,
        //                        2,-6.5,3,-2);
        for (int i = 0; i < 16; i++) test.vals[i] = 5*frand() - 2.5;
        mat4x4_determinant(test);
        vec4 solution = mat4x4_solve(test, v);
        print_vec4(solution);
        print_vec4(matrix_vec4(test, solution));
        mat4x4 inverse = mat4x4_inverse(test);
        printf("Inverse\n");
        print_mat4x4(inverse);
        print_mat4x4(mat4x4_multiply(test, inverse));
        getchar();
    }
#endif


    // Bezier demo.
    if (1) {
        BezierDemo *bd = BezierDemo_create(200,0,200, new_vec3(-100,60,-40), new_vec3(-30,100,30), new_vec3(30, 105, 6), new_vec3(50,95, 40), 0.2);
    }

    // NURBS demo.
    if (1) {
        #define nurbs_m 5
        #define nurbs_n 5
        vec3 points[nurbs_m * nurbs_m];
	float r = 28;
        for (int i = 0; i < nurbs_n; i++) {
            for (int j = 0; j < nurbs_m; j++) {
                points[nurbs_m * i + j] = new_vec3(50 * sin((1.0 / 27.0) * r * j), r * i, r * j);
                // points[i] = vec3_mul(vec3_normalize(new_vec3(frand()-0.5,frand()-0.5,frand()-0.5)), radius);
            }
        }
        Logic *g = NURBSDemo_create(0, 0, 500, points, nurbs_n, nurbs_m);
        NURBSDemo *nurbs = g->data;
    }
}





extern void loop_program(void)
{
    // paint_line_cv(Canvas2D, new_vec3(0.2, 0.2, 0), new_vec3(0.7,0.7,0), "k", 4);

    // Camera *camera;
    // for_aspect(Camera, _camera)
    //     camera = _camera;
    //     break;
    // end_for_aspect()
    // mat4x4 vp_matrix = Camera_vp_matrix(camera);
    // vec4 vanishing_plane = matrix_vec4(mat4x4_transpose(vp_matrix), new_vec4(0,0,0,1));
    // print_vec4(vanishing_plane);
}
