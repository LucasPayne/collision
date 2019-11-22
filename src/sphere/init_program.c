
void init_program(void)
{
    new_renderer_vertex_fragment(&basic_renderer, SHADERS_LOCATION "sphere.vert", SHADERS_LOCATION "sphere.frag");

    identity_matrix4x4f(&sphere_matrix);
    /* uniform_location_model_matrix = glGetUniformLocation(shader_program, "model_matrix"); */
    /* uniform_location_aspect_ratio = glGetUniformLocation(shader_program, "aspect_ratio"); */
    /* uniform_location_time = glGetUniformLocation(shader_program, "time"); */

    Mesh sphere_mesh;
    int tesselation_factor = 12;
    make_sphere(&sphere_mesh, 1.0, tesselation_factor);
    upload_and_free_mesh(&sphere_mesh_handle, &sphere_mesh);

    printf("Created mesh\n");
    print_mesh_handle(&sphere_mesh_handle);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
}
