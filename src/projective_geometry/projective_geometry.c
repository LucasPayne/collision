/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

typedef struct FrustumDemo_s {
} FrustumDemo;
void FrustumDemo_update(Logic *g)
{
    FrustumDemo *fd = g->data;
    Transform *t = Transform_get_a(g);
    mat4x4 matrix = Transform_matrix(t);
    painting_matrix(matrix);

    float radius = 100;
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
    EntityID e = new_gameobject(-100,0,0, 0,0,0, true);
    add_logic(e, FrustumDemo_update, FrustumDemo);
    ControlWidget_add(e, 10);
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
    paint_line_v(Canvas3D, vec3_zero(), a, new_vec4(0,0,0,0.3), 0.7);
    // paint_line_v(Canvas3D, vec3_zero(), b, new_vec4(0,0.2,0.13,0.4), 1);
    paint_line_v(Canvas3D, a, ap, new_vec4(0,0.2,0.13,0.4), 1);
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
        paint_grid_vv(Canvas3D, plane, new_vec4(0,0.8,0.21,0.63), enlarge*sm->grid_tess_x/3, enlarge*sm->grid_tess_y/3, 2);
        //paint_quad_vv(Canvas3D, plane, new_vec4(0,0.8,0.21,0.63));
    } else {
        paint_grid_vv(Canvas3D, plane, new_vec4(0,0.8,0.21,0.63), sm->grid_tess_x, sm->grid_tess_y, 2);
    }
    // for (int i = 0; i < 4; i++) plane[i] = base[i], plane[i].vals[1] += sm->plane_height;
    paint_sphere_v(Canvas3D, new_vec3(0,0,0), 1, new_vec4(0,0,0,1));
    // paint_sphere_v(Canvas3D, plane_origin, 1, new_vec4(0,0.5,0,1));
    // paint_line_v(Canvas3D, origin, plane_origin, new_vec4(0,0,0,1), 0.8);
    paint_grid_v(Canvas3D, base[0], base[1], base[2], base[3], new_vec4(0,0.3,0.9,0.5), sm->grid_tess_x, sm->grid_tess_y, 1);
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
    sm->grid_tess_x = 18;
    sm->grid_tess_y = 18;
    sm->plane_size = 120;

    Logic_add_input(g, INPUT_MOUSE_BUTTON, StraightModel_mouse_button_listener);

    EntityID eppc = new_gameobject(0,40,0, 0,0,0, true);
    Transform_get(eppc)->has_parent = true;
    Transform_get(eppc)->parent = Transform_get_a(g);
    ControlWidget *ppc = ControlWidget_add(eppc, 10);
    sm->plane_point_controller = eppc;

    return g;
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

typedef struct SplineDemo_s {
    int num_points;
    EntityID *point_controllers;
    StraightModel *sm;
} SplineDemo;
void SplineDemo_update(Logic *g)
{
    SplineDemo *sd = g->data;
    painting_matrix(Transform_get_matrix_a(g));
    for (int i = 0; i < sd->num_points-1; i++) {
        draw_projected_segment(sd->sm, Transform_get_position(sd->point_controllers[i]), Transform_get_position(sd->point_controllers[i+1]), new_vec4(1,0,1,1), new_vec4(1,0,1,1), 2);
    }
    painting_matrix_reset();
}

extern void init_program(void)
{
    // create_key_camera_man(0,0,0,  0,0,0);
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
            body->material = Material_create("Materials/textured_phong");
            material_set_texture_path(resource_data(Material, body->material), "diffuse_map", "Textures/brick_wall");
            // body->material = Material_create("Materials/textured_phong_shadows_normal_mapped");
            // material_set_texture_path(resource_data(Material, body->material), "diffuse_map", "Textures/brick_wall");
            // material_set_texture_path(resource_data(Material, body->material), "normal_map", "Textures/brick_wall_normal");
        }
    }



    FrustumDemo_create(0,0,0);
    {
    Logic *smg = StraightModel_add(180,0,0, 100, 100, 20);
    StraightModel *sm = smg->data;
    sm->plane_normal = vec3_normalize(new_vec3(0.5,1,0));
    StraightModelDemo *sd = add_logic(smg->entity_id, StraightModelDemo_update, StraightModelDemo)->data;
    sd->sm = sm;
    }

    {
    Logic *smg = StraightModel_add(400,0,0, 100, 100, 20);
    StraightModel *sm = smg->data;
    Transform *t = Transform_get_a(smg);
    Logic *g = add_logic(smg->entity_id, SplineDemo_update, SplineDemo);
    SplineDemo *sd = g->data;
    sd->sm = sm;
    sd->num_points = 8;
    sd->point_controllers = malloc(sizeof(EntityID) * sd->num_points);
    mem_check(sd->point_controllers);
    for (int i = 0; i < sd->num_points; i++) {
        EntityID cwe = new_gameobject(0,0,0,0,0,0, true);
        Transform *cwt = Transform_get(cwe);
        cwt->has_parent = true;
        cwt->parent = t;
        ControlWidget_add(cwe, 8);
        float theta = 2*M_PI*i*1.0/sd->num_points;
        Transform_set_position(cwt, new_vec3(30*cos(theta), 50, 30*sin(theta)));
        //Transform_set_position(cwt, new_vec3(40*frand(),40*frand(),40*frand()));
        sd->point_controllers[i] = cwe;
    }
    }
    //---for some reason if this is the first entity created, nothing can be seen.
    Player_create_default(0,0,0, 0,0);
}
