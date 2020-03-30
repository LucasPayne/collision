/*--------------------------------------------------------------------------------
    Game-renderer module. This is the rendering stuff that depends on the gameobject
    system. Any general utilities for working with OpenGL and glsl, material, texture, and geometry resources,
    etc., are part of the rendering module. The game-renderer is a client of these rendering facilities.
--------------------------------------------------------------------------------*/
#include "Engine.h"

void init_game_renderer(void)
{
    painting_init();
    init_shadows();
}

void render_body_with_material(mat4x4 vp_matrix, Body *body, Material *material)
{
    Transform *transform = get_sibling_aspect(body, Transform);
    Geometry *mesh = resource_data(Geometry, body->geometry);
    // Form the mvp matrix and upload it, along with other Standard3D information.
    Matrix4x4f model_matrix = Transform_matrix(transform);
    // //---This body rescaling is a hack.
    // for (int i = 0; i < 3; i++) {
    //     for (int j = 0; j < 3; j++) {
    //         model_matrix.vals[4*i + j] *= body->scale;
    //     }
    // }
    Matrix4x4f mvp_matrix = vp_matrix;
    right_multiply_matrix4x4f(&mvp_matrix, &model_matrix);
    set_uniform_vec3(Standard3D, model_position, new_vec3(transform->x, transform->y, transform->z));
    set_uniform_mat4x4(Standard3D, model_matrix.vals, model_matrix.vals);
    Matrix4x4f normal_matrix;
    euler_rotation_matrix4x4f(&normal_matrix, transform->theta_x, transform->theta_y, transform->theta_z);
    set_uniform_mat4x4(Standard3D, normal_matrix.vals, normal_matrix.vals); // assuming only rigid transformations.
    set_uniform_mat4x4(Standard3D, mvp_matrix.vals, mvp_matrix.vals);
    gm_draw(*mesh, material);
}
void render_body(mat4x4 vp_matrix, Body *body)
{
    Material *material = resource_data(Material, body->material);
    render_body_with_material(vp_matrix, body, material);
}

mat4x4 Camera_prepare(Camera *camera)
{
    // Prepare the rendering context for rendering with this camera.
    glEnable(GL_SCISSOR_TEST);
    float camera_viewport[4];
    camera_viewport[0] = g_subwindow_blx + camera->blx * g_window_width;
    camera_viewport[1] = g_subwindow_bly + camera->bly * g_window_height;
    camera_viewport[2] = (camera->trx - camera->blx) * g_window_width;
    camera_viewport[3] = (camera->try - camera->bly) * g_window_height;
    glScissor(camera_viewport[0], camera_viewport[1], camera_viewport[2], camera_viewport[3]);
    if (camera->override_bg_color) {
        glClearColor(camera->bg_color.vals[0], camera->bg_color.vals[1], camera->bg_color.vals[2], camera->bg_color.vals[3]);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(g_bg_color[0], g_bg_color[1], g_bg_color[2], g_bg_color[3]);
    }
    // Form the view-projection matrix.
    Transform *camera_transform = get_sibling_aspect(camera, Transform);
    // Account for the camera's target rectangle.
    float x_scale = camera->trx - camera->blx;
    float y_scale = camera->try - camera->bly;
    float x_shift = 2*camera->blx - 1 + x_scale;
    float y_shift = 2*camera->bly - 1 + y_scale;
    mat4x4 vp_matrix = {{
        x_scale, 0,       0, 0,
        0,       y_scale, 0, 0,
        0,       0,       1, 0,
        x_shift, y_shift, 0, 1,
    }};
    mat4x4 view_matrix = invert_rigid_mat4x4(Transform_matrix(camera_transform));
    right_multiply_matrix4x4f(&vp_matrix, &camera->projection_matrix);
    right_multiply_matrix4x4f(&vp_matrix, &view_matrix);
    
    // Upload the camera position and direction, and other information.
    set_uniform_vec3(Standard3D, camera_position, new_vec3(camera_transform->x, camera_transform->y, camera_transform->z));
    set_uniform_vec3(Standard3D, camera_direction, vec3_neg(Transform_forward(camera_transform)));
    // Upload camera parameters.
    set_uniform_float(Standard3D, near_plane, camera->plane_n);
    set_uniform_float(Standard3D, far_plane, camera->plane_f);
    
    // Upload the uniform half-vectors for directional lights. This depends on the camera, and saves recomputation of the half-vector per-pixel,
    // since in the case of directional lights this vector is constant.
    int directional_light_index = 0; //---may be artifacts due to the desynchronization of the index of the directional light through each frame when a light changes.
    for_aspect(DirectionalLight, directional_light)
        vec3 direction = DirectionalLight_direction(directional_light);
        // Both the directional light direction and the camera forward vector are unit length, so their sum gives a half vector, then this is normalized.
        vec3 forward = vec3_neg(Transform_forward(camera_transform));
        float hx = forward.vals[0] + direction.vals[0];
        float hy = forward.vals[1] + direction.vals[1];
        float hz = forward.vals[2] + direction.vals[2];
        float inv_length = 1/sqrt(hx*hx + hy*hy + hz*hz);
        hx *= inv_length;
        hy *= inv_length;
        hz *= inv_length;
        set_uniform_vec3(Lights, directional_lights[directional_light_index].half_vector, new_vec3(hx, hy, hz));
        directional_light_index ++;
    end_for_aspect()

    return vp_matrix;
}

void render(void)
{
    set_uniform_float(StandardLoopWindow, time, time);

// int index = 0; ////////testing
    for_aspect(Camera, camera)
        do_shadows(camera);
        // if (index++ == 0) do_shadows(camera); ////////testing
        mat4x4 vp_matrix = Camera_prepare(camera);
        // Render each body.
        for_aspect(Body, body)
            if (body->visible) render_body(vp_matrix, body);
        end_for_aspect()
        // Draw the buffered paint (in global coordinates).
        set_uniform_mat4x4(Standard3D, mvp_matrix.vals, vp_matrix.vals);
        glCullFace(GL_NONE);
        glDisable(GL_CULL_FACE);
        painting_draw(Canvas3D);
        glEnable(GL_CULL_FACE);
        glCullFace(g_cull_mode);
    end_for_aspect()
    // Flush the standard 3D paint canvas.
    painting_flush(Canvas3D);
    render_paint2d();
    painting_flush(Canvas2D);

    // Render text.
    for_aspect(Camera, camera)
        float aspect_ratio = (camera->plane_t - camera->plane_b) / (camera->plane_r - camera->plane_l);
        for_aspect(Text, text)
            vec3 position = Transform_position(get_sibling_aspect(text, Transform));
            mat4x4 text_matrix;
            if (text->type == TextOriented || text->type == TextOrientedFixed) {
                // Oriented text is rendered toward the camera, either of a fixed size or size up to the depth the text is at.
                mat4x4 vp_matrix = Camera_prepare(camera);
                vec4 transformed = matrix_vec4(&vp_matrix, vec3_to_vec4(position));
                float screen_x = transformed.vals[0] / transformed.vals[3];
                float screen_y = transformed.vals[1] / transformed.vals[3];
                float depth = transformed.vals[2];
                // printf("(%.2f, %.2f, depth: %.2f)\n", screen_x, screen_y, depth);
                float scale = text->scale;
                if (text->type == TextOriented) {
                    if (depth <= 0.0001) continue;
                    scale /= depth;
                }

                // float test_quad_size = 0.05;
                // paint2d_quad_c(0.5*(screen_x-test_quad_size)+0.5,0.5*(screen_y-test_quad_size)+0.5,
                //                0.5*(screen_x+test_quad_size)+0.5,0.5*(screen_y-test_quad_size)+0.5,
                //                0.5*(screen_x+test_quad_size)+0.5,0.5*(screen_y+test_quad_size)+0.5,
                //                0.5*(screen_x-test_quad_size)+0.5,0.5*(screen_y+test_quad_size)+0.5, "b");
                mat4x4 M = {{
                    scale,    0,                    0, 0,
                    0,        scale / aspect_ratio, 0, 0,
                    0,        0,                    1, 0,
                    screen_x, screen_y,             0, 1,
                }};
                text_matrix = M;
            } else if (text->type == Text2D) {
                // Text2D text uses the x and y components of the transform to determine the screen position, in terms of 2D coordinates (0,0) at bottom-left to (1,1) at top-right.
                float scale = text->scale;
                mat4x4 M = {{
                    scale,    0,                        0, 0,
                    0,        scale / aspect_ratio,     0, 0,
                    0,        0,                        1, 0,
                    2*position.vals[0]-1, 2*position.vals[1]-1, 0, 1,
                }};
                text_matrix = M;
            } else {
                fprintf(stderr, ERROR_ALERT "Attempted to render text which does not have a text-type which is accounted for.\n");
                exit(EXIT_FAILURE);
            }
            
           // else if (text->type == TextPlaced) {
           //     // "Placed" text has an orientation determined by its transform.
           // }

            float test_quad_size = 0.05;
            // paint2d_quad_c(0.5*(screen_x-test_quad_size)+0.5,0.5*(screen_y-test_quad_size)+0.5,
            //                0.5*(screen_x+test_quad_size)+0.5,0.5*(screen_y-test_quad_size)+0.5,
            //                0.5*(screen_x+test_quad_size)+0.5,0.5*(screen_y+test_quad_size)+0.5,
            //                0.5*(screen_x-test_quad_size)+0.5,0.5*(screen_y+test_quad_size)+0.5, "b");
            Text_render(text_matrix, text);
        end_for_aspect()
    end_for_aspect()

}
void render_paint2d()
{
    // Draw the 2D overlay paint. ----make this an actual overlay.
    bool culling;
    glGetIntegerv(GL_CULL_FACE, &culling);
    glDisable(GL_CULL_FACE);
    mat4x4 overlay_matrix = matrix_paint2d();
    set_uniform_mat4x4(Standard3D, mvp_matrix.vals, overlay_matrix.vals);
    painting_draw(Canvas2D);
    if (culling) glEnable(GL_CULL_FACE);
}
