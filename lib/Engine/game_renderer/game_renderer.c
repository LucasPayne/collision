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
    //---This body rescaling is a hack.
    for (int i = 0; i < 3; i++) {
	for (int j = 0; j < 3; j++) {
	    model_matrix.vals[4*i + j] *= body->scale;
	}
    }
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

    // Form the view-projection matrix.
    Transform *camera_transform = get_sibling_aspect(camera, Transform);
    Matrix4x4f view_matrix = invert_rigid_mat4x4(Transform_matrix(camera_transform));
    Matrix4x4f vp_matrix = camera->projection_matrix;
    right_multiply_matrix4x4f(&vp_matrix, &view_matrix);
    
    // Upload the camera position and direction, and other information.
    set_uniform_vec3(Standard3D, camera_position, new_vec3(camera_transform->x, camera_transform->y, camera_transform->z));
    vec4 camera_forward_vector = matrix_vec4(&view_matrix, new_vec4(0,0,1,1));
    set_uniform_vec3(Standard3D, camera_direction, *((vec3 *) &camera_forward_vector)); //... get better matrix/vector routines.
    // Upload camera parameters.
    set_uniform_float(Standard3D, near_plane, camera->plane_n);
    set_uniform_float(Standard3D, far_plane, camera->plane_f);
    
    // Upload the uniform half-vectors for directional lights. This depends on the camera, and saves recomputation of the half-vector per-pixel,
    // since in the case of directional lights this vector is constant.
    int directional_light_index = 0; //---may be artifacts due to the desynchronization of the index of the directional light through each frame when a light changes.
    for_aspect(DirectionalLight, directional_light)
        vec3 direction = DirectionalLight_direction(directional_light);
        // Both the directional light direction and the camera forward vector are unit length, so their sum gives a half vector, then this is normalized.
        float hx = camera_forward_vector.vals[0] + direction.vals[0];
        float hy = camera_forward_vector.vals[1] + direction.vals[1];
        float hz = camera_forward_vector.vals[2] + direction.vals[2];
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

    do_shadows();
    for_aspect(Camera, camera)
        mat4x4 vp_matrix = Camera_prepare(camera);
        // Render each body.
        for_aspect(Body, body)
            render_body(vp_matrix, body);
        end_for_aspect()
        // Draw the buffered paint (in global coordinates).
        set_uniform_mat4x4(Standard3D, mvp_matrix.vals, vp_matrix.vals);
        painting_draw(Canvas3D);
    end_for_aspect()
    // Flush the standard 3D paint canvas.
    painting_flush(Canvas3D);

    // Draw the 2D overlay paint. ----make this an actual overlay.
    bool culling;
    glGetIntegerv(GL_CULL_FACE, &culling);
    glDisable(GL_CULL_FACE);
    mat4x4 overlay_matrix = matrix_paint2d();
    set_uniform_mat4x4(Standard3D, mvp_matrix.vals, overlay_matrix.vals);
    painting_draw(Canvas2D);
    if (culling) glEnable(GL_CULL_FACE);
}

