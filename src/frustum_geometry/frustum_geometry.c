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
    paint_line_cv(pos, near_p, "b");
    paint_line_cv(near_p, far_p, "r");

    char *colors[] = {"r", "g", "b", "y"};

    for (int segment = 0; segment < 4; segment++) {
        float along = -n - (f - n) * segment/4.0 * f/n;
        float along_to = -n - (f - n) * (segment + 1)/4.0 * f/n;

        near_p = vec3_add(pos, vec3_mul(Transform_forward(transform), along));
        far_p =  vec3_add(pos, vec3_mul(Transform_forward(transform), along_to));

        vec3 near_quad[] = {
            vec3_add(near_p, Transform_relative_direction(transform, vec3_mul(new_vec3(l, t, 0), along/n))),
            vec3_add(near_p, Transform_relative_direction(transform, vec3_mul(new_vec3(l, b, 0), along/n))),
            vec3_add(near_p, Transform_relative_direction(transform, vec3_mul(new_vec3(r, b, 0), along/n))),
            vec3_add(near_p, Transform_relative_direction(transform, vec3_mul(new_vec3(r, t, 0), along/n))),
        };
        vec3 far_quad[] = {
            vec3_add(far_p, Transform_relative_direction(transform, vec3_mul(new_vec3(l, t, 0),  along_to/n))),
            vec3_add(far_p, Transform_relative_direction(transform, vec3_mul(new_vec3(l, b, 0),  along_to/n))),
            vec3_add(far_p, Transform_relative_direction(transform, vec3_mul(new_vec3(r, b, 0),  along_to/n))),
            vec3_add(far_p, Transform_relative_direction(transform, vec3_mul(new_vec3(r, t, 0),  along_to/n))),
        };
        for (int i = 0; i < 4; i++) {
            paint_line_cv(near_quad[i], near_quad[(i+1)%4], colors[segment]);
            paint_line_cv(far_quad[i], far_quad[(i+1)%4], colors[segment]);
            paint_line_cv(near_quad[i], far_quad[i], colors[segment]);
        }
    
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
                    }
                }
                p++;
            }
            // Transform this box to world space.
	    mat4x4 light_to_world = Transform_matrix(get_sibling_aspect(light, Transform));
            for (int i = 0; i < 8; i++) {
                box_points[i] = mat4x4_vec3(&light_to_world, box_points[i]);
            }
            // Draw the box.
            for (int j = 0; j < 2; j++) {
                for (int i = 0; i < 4; i++) {
                    paint_line_cv(box_points[4*j + i], box_points[4*j + i%4], "k");
                }
            }
            for (int i = 0; i < 4; i++) {
                paint_line_cv(box_points[i], box_points[i + 4], "k");
            }
        end_for_aspect()
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
    for_aspect(Camera, camera)
        draw_frustum(camera);
        break;
    end_for_aspect()
}
extern void close_program(void)
{
}
