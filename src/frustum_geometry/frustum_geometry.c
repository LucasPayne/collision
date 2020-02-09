/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

void draw_frustum(Camera *camera)
{
    float n, f, b, t, l, r;
    n = camera->plane_n;
    f = camera->plane_f;
    b = camera->plane_b;
    t = camera->plane_t;
    l = camera->plane_l;
    r = camera->plane_r;

    Transform *transform = get_sibling_aspect(camera, Transform);
    vec3 pos = Transform_position(transform);
    vec3 near_p = vec3_add(pos, vec3_mul(Transform_forward(transform), -n));
    vec3 far_p =  vec3_add(pos, vec3_mul(Transform_forward(transform), -f));
    paint_line_cv(Canvas3D, pos, near_p, "b", 4);
    paint_line_cv(Canvas3D, near_p, far_p, "r", 4);

    vec4 colors[] = {   
        new_vec4(1,0,0,1),
        new_vec4(0,1,0,1),
        new_vec4(0,0,1,1),
        new_vec4(0,1,1,1),
    };

    for (int segment = 0; segment < 4; segment++) {
        float along = -n - (f - n) * segment/4.0;
        float along_to = -n - (f - n) * (segment + 1)/4.0;

        near_p = vec3_add(pos, vec3_mul(Transform_forward(transform), along));
        far_p =  vec3_add(pos, vec3_mul(Transform_forward(transform), along_to));
        //---2* ?
        vec3 frustum_points[] = {
            vec3_add(near_p, Transform_relative_direction(transform, vec3_mul(new_vec3(l, t, 0), along + n))),
            vec3_add(near_p, Transform_relative_direction(transform, vec3_mul(new_vec3(l, b, 0), along + n))),
            vec3_add(near_p, Transform_relative_direction(transform, vec3_mul(new_vec3(r, b, 0), along + n))),
            vec3_add(near_p, Transform_relative_direction(transform, vec3_mul(new_vec3(r, t, 0), along + n))),
            vec3_add(far_p, Transform_relative_direction(transform, vec3_mul(new_vec3(l, t, 0),  along_to + n))),
            vec3_add(far_p, Transform_relative_direction(transform, vec3_mul(new_vec3(l, b, 0),  along_to + n))),
            vec3_add(far_p, Transform_relative_direction(transform, vec3_mul(new_vec3(r, b, 0),  along_to + n))),
            vec3_add(far_p, Transform_relative_direction(transform, vec3_mul(new_vec3(r, t, 0),  along_to + n))),
        };
        vec3 *near_quad = frustum_points;
        vec3 *far_quad = frustum_points + 4;

        paint_box_v(Canvas3D, frustum_points, color_fade(colors[segment], 0.5));
        
        for (int i = 0; i < 4; i++) {
            paint_line_v(Canvas3D, near_quad[i], near_quad[(i+1)%4], colors[segment], 2);
            paint_line_v(Canvas3D, far_quad[i], far_quad[(i+1)%4], colors[segment], 2);
            paint_line_v(Canvas3D, near_quad[i], far_quad[i], colors[segment], 2);
        }
    #if 1
        for_aspect(DirectionalLight, light)
            mat4x4 light_matrix = invert_rigid_mat4x4(Transform_matrix(get_sibling_aspect(light, Transform)));
            // Transform frustum segment to light space.
            vec3 light_frustum[8];
            for (int i = 0; i < 4; i++) {
                light_frustum[i] = mat4x4_vec3(&light_matrix, near_quad[i]);
                light_frustum[i + 4] = mat4x4_vec3(&light_matrix, far_quad[i]);
            }
            // Find the axis-aligned bounding box of the frustum segment in light coordinates.
            // Find the minimum and maximum corners.
            vec3 box_corners[2] = { light_frustum[0], light_frustum[0] };
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 3; j++) {
                    if (light_frustum[i].vals[j] < box_corners[0].vals[j]) box_corners[0].vals[j] = light_frustum[i].vals[j];
                    if (light_frustum[i].vals[j] > box_corners[1].vals[j]) box_corners[1].vals[j] = light_frustum[i].vals[j];
                }
            }
            // Use these to form all points of the box.
            vec3 box_points[8];
            int p = 0;
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    for (int k = 0; k < 2; k++) {
                        box_points[p] = new_vec3(box_corners[i].vals[0], box_corners[j].vals[1], box_corners[k].vals[2]);
                        p++;
                    }
                }
            }
            // Transform this box to world space.
	    mat4x4 light_to_world = Transform_matrix(get_sibling_aspect(light, Transform));
            for (int i = 0; i < 8; i++) {
                box_points[i] = mat4x4_vec3(&light_to_world, box_points[i]);
            }
            // Draw the box.
            for (int i = 0; i < 4; i++) {
                paint_line_v(Canvas3D, box_points[2*i], box_points[2*i+1], color_mul(colors[segment], 0.5), 1);
                paint_line_v(Canvas3D, box_points[i], box_points[i+4], color_mul(colors[segment], 0.5), 1);
            }
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    paint_line_v(Canvas3D, box_points[i + 4*j], box_points[i + 4*j + 2], color_mul(colors[segment], 0.5), 1);
                }
            }
        end_for_aspect()
#endif
    }
}
void swap_camera(void)
{
    static int camera_num = 0;
    int i = 0;
    for_aspect(Camera, camera)
        Input *input = get_sibling_aspect(camera, Input);
        Logic *logic = get_sibling_aspect(camera, Logic);
        if (i == camera_num) {
            input->listening = true;
            logic->updating = true;
        }
        else {
            input->listening = false;
            logic->updating = false;
        }
        i ++;
    end_for_aspect()
    camera_num = (camera_num + 1) % i;
}
extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS && key == GLFW_KEY_N) {
        swap_camera();
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
    Camera *camera_1 = get_aspect_type(create_key_camera_man(0,0,0,  0,0,0), Camera);
#if 1
    camera_1->override_bg_color = true;
    camera_1->bg_color = new_vec4(0.8,1,0.8,1);
    camera_1->bly = 0.25;
    camera_1->try = 0.75;
    camera_1->trx = 0.5;
    Camera *camera_2 = get_aspect_type(create_key_camera_man(0,0,200,  0,0,0), Camera);
    camera_2->override_bg_color = true;
    camera_2->bg_color = new_vec4(1,0.8,1,1);
    camera_2->blx = 0.5;
    camera_2->bly = 0.25;
    camera_2->try = 0.75;
#endif

#if 0
    EntityID e = new_entity(4);
    Transform_set(entity_add_aspect(e, Transform), 0,0,-200, 0,0,0);
    Body *body = entity_add_aspect(e, Body);
    body->scale = 100;
    body->geometry = new_resource_handle(Geometry, "Models/block");
    body->material = Material_create("Materials/texture");
    material_set_texture_path(resource_data(Material, body->material), "diffuse_map", "Textures/mario/sand_bricks");
#endif

    open_scene(g_scenes, "block_on_floor");
    test_directional_light_controlled();
    
    swap_camera();
}
extern void loop_program(void)
{
#if 1
    for_aspect(Camera, camera)
        draw_frustum(camera);
        break;
    end_for_aspect()
#endif

    for_aspect(DirectionalLight, light)
        Transform *t = get_sibling_aspect(light, Transform);
        vec3 dir = DirectionalLight_direction(light);
        vec3 pos = Transform_position(t);
        paint_line_cv(Canvas3D, pos, vec3_add(pos, vec3_mul(dir, 10)), "r", 2);

        vec3 near_plane[] = {
            Transform_relative_position(t, new_vec3(-light->shadow_width/2,-light->shadow_height/2,0)),
            Transform_relative_position(t, new_vec3(light->shadow_width/2,-light->shadow_height/2,0)),
            Transform_relative_position(t, new_vec3(light->shadow_width/2,light->shadow_height/2,0)),
            Transform_relative_position(t, new_vec3(-light->shadow_width/2,light->shadow_height/2,0)),
        };
        vec3 far_plane[] = {
            Transform_relative_position(t, new_vec3(-light->shadow_width/2,-light->shadow_height/2,light->shadow_depth)),
            Transform_relative_position(t, new_vec3(light->shadow_width/2,-light->shadow_height/2,light->shadow_depth)),
            Transform_relative_position(t, new_vec3(light->shadow_width/2,light->shadow_height/2,light->shadow_depth)),
            Transform_relative_position(t, new_vec3(-light->shadow_width/2,light->shadow_height/2,light->shadow_depth)),
        };
        // paint_loop_c(Canvas3D, (float *) near_plane, 4, "y");
        // paint_loop_c(Canvas3D, (float *) far_plane, 4, "y");
        for (int i = 0; i < 4; i++) paint_line_cv(Canvas3D, near_plane[i], far_plane[i], "g", 2);
        
    end_for_aspect()
}
extern void close_program(void)
{
}
