
void init_program(void)
{
    EntityID camera_man = new_entity(2);
    Transform_set(entity_add_aspect(camera_man, Transform), 0,0,0,0,0,0);
    Camera *camera = entity_add_aspect(camera_man, Camera);
    Camera_init(camera, ASPECT_RATIO, 1, 0.9, 10);

    EntityID cube = new_entity(2);
    Transform_set(entity_add_aspect(cube, Transform), 0,0,3,1,1,1);
    Body *body = entity_add_aspect(cube, Body);
    body->geometry = new_resource_handle(Geometry, "Models/cube");
    Material *mat = oneoff_resource(Material, body->material);
    mat->material_type = new_resource_handle(MaterialType, "Materials/tinted_texture");
    material_set_property_vec4(mat, "flat_color", new_vec4(frand(), frand(), frand(), 1));
    material_set_texture_path(mat, "diffuse_map", "Textures/stone_bricks");
}
void loop(void)
{
    for_aspect(Camera, camera)
        Transform *camera_transform = get_sibling_aspect(camera, Transform);
        Matrix4x4f view_matrix = Transform_matrix(camera_transform);
        Matrix4x4f vp_matrix = camera->projection_matrix;
        right_multiply_matrix4x4f(&vp_matrix, &view_matrix);

        for_aspect(Body, body)
            Transform *transform = get_sibling_aspect(body, Transform);
            Geometry *mesh = resource_data(Geometry, body->geometry);
            Material *material = resource_data(Material, body->material);
            Matrix4x4f model_matrix = Transform_matrix(transform);
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    model_matrix.vals[4*i + j] *= body->scale;
                }
            }
            Matrix4x4f mvp_matrix = vp_matrix;
            right_multiply_matrix4x4f(&mvp_matrix, &model_matrix);

            set_uniform_mat4x4(Standard3D, mvp_matrix.vals, mvp_matrix.vals);
            set_uniform_float(StandardLoopWindow, TEST_VALUE, camera_transform->theta_x);

            gm_draw(*mesh, material);
        end_for_aspect()
    end_for_aspect()
}
