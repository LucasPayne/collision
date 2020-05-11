/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

// Just put it really far away.
vec3 point_at_infinity(vec3 direction)
{
    return vec3_mul(direction, 10000);
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

    // vec3 mapped_points[8]; // near quad, far quad

    bool lerping;
    float lerp_distances[4];
    float lerp_distance_fifth_point;
} FrustumDemo;

void FrustumDemo_key_listener(Logic *g, int key, int action, int mods)
{
    FrustumDemo *fd = g->data;
    Transform *t = Transform_get_a(g);

    if (action == GLFW_PRESS && key == GLFW_KEY_B) {
        fd->lerping = true;
        for (int i = 0; i < 4; i++) {
            fd->lerp_to_points[i] = fd->box_points[mapped_indices[i]];
            fd->lerp_distances[i] = vec3_length(vec3_sub(fd->mapped_points[i], fd->lerp_to_points[i]));
        }
        fd->lerp_to_fifth_point = fd->box_fifth_point;
        fd->lerp_distance_fifth_point = vec3_length(vec3_sub(fd->fifth_mapped_point, fd->lerp_to_fifth_point));
    }
    if (action == GLFW_PRESS && key == GLFW_KEY_F) {
        fd->lerping = true;
        for (int i = 0; i < 4; i++) {
            fd->lerp_to_points[i] = fd->frustum_points[mapped_indices[i]];
            fd->lerp_distances[i] = vec3_length(vec3_sub(fd->mapped_points[i], fd->lerp_to_points[i]));
        }
        fd->lerp_to_fifth_point = fd->frustum_fifth_point;
        fd->lerp_distance_fifth_point = vec3_length(vec3_sub(fd->fifth_mapped_point, fd->lerp_to_fifth_point));
    }
}

void FrustumDemo_update(Logic *g)
{
    FrustumDemo *fd = g->data;
    Transform *t = Transform_get_a(g);
    mat4x4 matrix = Transform_matrix(t);
    painting_matrix(matrix);

    float lerp_seconds = 0.6;
    if (fd->lerping) {
        for (int i = 0; i < 4; i++) {
            vec3 m = fd->mapped_points[i];
            vec3 t = fd->lerp_to_points[i];
            float d = vec3_square_length(vec3_sub(t, m));
            if (d < (fd->lerp_distances[i] / 50)*(fd->lerp_distances[i] / 50)) {
                fd->mapped_points[i] = t;
            } else {
                fd->mapped_points[i] = vec3_add(m, vec3_mul(vec3_normalize(vec3_sub(t, m)), fd->lerp_distances[i]*dt / lerp_seconds));
            }
        }
        vec3 m = fd->fifth_mapped_point;
        vec3 t = fd->lerp_to_fifth_point;
        float d = vec3_square_length(vec3_sub(t, m));
        if (d < (fd->lerp_distance_fifth_point / 50)*(fd->lerp_distance_fifth_point / 50)) {
            fd->fifth_mapped_point = t;
        } else {
            fd->fifth_mapped_point = vec3_add(m, vec3_mul(vec3_normalize(vec3_sub(t, m)), fd->lerp_distance_fifth_point*dt / lerp_seconds));
        }
    }

    paint_wireframe_box_v(Canvas3D, fd->box_points, new_vec4(0.5,0.5,0.5,1), 1.3);
    paint_wireframe_box_v(Canvas3D, fd->frustum_points, new_vec4(0.4,0.4,0.4,1), 1.3);

    for (int i = 0; i < 4; i++) {
        paint_line_v(Canvas3D, fd->box_points[mapped_indices[i]], fd->mapped_points[i], new_vec4(0.9,0,0.7,0.9), 0.8);
        paint_points(Canvas3D, &fd->mapped_points[i], 1, 0.8,0.2,0.8,1, 18);
    }
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
    int tess = 10;
    paint_grid_vv(Canvas3D, xz_square, color1, tess, tess, 0.8);
    paint_grid_vv(Canvas3D, xy_square, color2, tess, tess, 0.8);
    paint_grid_vv(Canvas3D, yz_square, color3, tess, tess, 0.8);
    for (int i = 0; i < 4; i++) {
        paint_line_v(Canvas3D, xz_square[i], xz_square[(i+1)%4], color1, 2);
        paint_line_v(Canvas3D, xy_square[i], xy_square[(i+1)%4], color2, 2);
        paint_line_v(Canvas3D, yz_square[i], yz_square[(i+1)%4], color3, 2);
    }
    paint_line_v(Canvas3D, new_vec3(-radius, 0,0), new_vec3(radius,0,0), new_vec4(0,0,0,0.6), 1);
    paint_line_v(Canvas3D, new_vec3(0, -radius, 0), new_vec3(0,radius,0), new_vec4(0,0,0,0.6), 1);
    paint_line_v(Canvas3D, new_vec3(0,0,-radius), new_vec3(0,0,radius), new_vec4(0,0,0,0.6), 1);
}
EntityID FrustumDemo_create(float x, float y, float z)
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
        fd->mapped_points[i] = fd->frustum_points[mapped_indices[i]];
    }
    fd->fifth_mapped_point = vec3_zero();
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
        draw_projected_segment(sm, new_vec3(30*cos(tt+t), 40, 13*sin(2*t)), new_vec3(30*cos(tt+tp), 40, 13*sin(2*tp)), new_vec4(0,0,0,transparency), new_vec4(1,0,0,transparency), 2.8);
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
        draw_projected_segment(sd->sm, SD_pos(sd, i), SD_pos(sd, i), new_vec4(1,0,1,1), new_vec4(0,0,0,1), 4);
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

void SplineDemo3_update(Logic *g)
{
    // Cubic spline.
    const mat4x4 cubic_bspline_to_cubic_bernstein = {{
        0, 0, 0, 1.0/6.0,
        1.0/6.0, 1.0/3.0, 2.0/3.0, 2.0/3.0,
        2.0/3.0, 2.0/3.0, 1.0/3.0, 1.0/6.0,
        1.0/6.0, 0,0,0,
    }};
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

extern void init_program(void)
{
#if 0
{
    EntityID light = new_entity(4);
    Transform *t = entity_add_aspect(light, Transform);
    Transform_set(t, 0,200,0,  M_PI/2+0.3,0,0);
    t->euler_controlled = true;
    DirectionalLight_init(entity_add_aspect(light, DirectionalLight), 1,0.7,0.7,1,  400,400,500);
}
    int bunny_square_root = 1;
    for (int i = 0; i < bunny_square_root; i++) {
        for (int j = 0; j < bunny_square_root; j++) {
            EntityID e = new_entity(4);
            float apart = 400;
            Transform *t = add_aspect(e, Transform);
            Transform_set(t, 200+i*apart,-10,200+j*apart, 0,0,0);
            t->scale = 400;
            
            Body *body = add_aspect(e, Body);
            //body->scale = 400;
            body->visible = true;
            body->geometry = new_resource_handle(Geometry, "Models/stanford_bunny -a");
#if 0
            body->material = Material_create("Materials/textured_phong");
            material_set_texture_path(resource_data(Material, body->material), "diffuse_map", "Textures/brick_wall");
#else
            body->material = Material_create("Materials/textured_phong_shadows_normal_mapped");
            material_set_texture_path(resource_data(Material, body->material), "diffuse_map", "Textures/brick_wall");
            material_set_texture_path(resource_data(Material, body->material), "normal_map", "Textures/brick_wall_normal");
#endif
        }
    }
#endif


#if 1
    FrustumDemo_create(-300,40,0);
#endif
#if 0
    {
    Logic *smg = StraightModel_add(180,0,0, 100, 100, 20);
    StraightModel *sm = smg->data;
    sm->plane_normal = vec3_normalize(new_vec3(0.5,1,0));
    StraightModelDemo *sd = add_logic(smg->entity_id, StraightModelDemo_update, StraightModelDemo)->data;
    sd->sm = sm;
    }
#endif

#if 0
    {
    vec3 points[5];
    get_regular_polygon(points, 5, new_vec3(0,50,0), new_vec3(0,1,0), new_vec3(1,0,0), 40);
    SplineDemo_create(400,0,0, points, 5, SplineDemo1_update);
    }
#endif
    //---for some reason if this is the first entity created, nothing can be seen.
    PlayerController *player = Player_create_default(0,70,200, 0,0);
    player->scrollable_speed = false;
#if 0
    {
    vec3 points[5];
    get_regular_polygon(points, 4, new_vec3(0,50,0), new_vec3(0,1,0), new_vec3(1,0,0), 40);
    //SplineDemo_create(400,0,220, points, 5, SplineDemo2_update);
    SplineDemo_create(0,0,0, points, 5, SplineDemo2_update);
    }
#endif
    {
    #define N 6
    vec3 points[N];
    get_regular_polygon(points, N, new_vec3(0,75,0), new_vec3(0,1,0), new_vec3(1,0,0), 55);
    //SplineDemo_create(400,0,220, points, 5, SplineDemo2_update);
    SplineDemo_create(0,0,0, points, N, SplineDemo3_update);
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
