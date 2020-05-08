
void loop(GLFWwindow *window)
{
    /* glUniform1f(uniform_location_aspect_ratio, ASPECT_RATIO); */
    /* glUniform1f(uniform_location_time, time()); */

    if (arrow_key_down(Up)) {
        translate_matrix4x4f(&sphere_matrix, 0, 0, -1 * dt());
    }
    if (arrow_key_down(Down)) {
        translate_matrix4x4f(&sphere_matrix, 0, 0, 1 * dt());
    }
    if (arrow_key_down(Left)) {
        translate_matrix4x4f(&sphere_matrix, -1 * dt(), 0, 0);
        mat4x4 rot;
        /* euler_rotation_mat4x4(&rot, -0.01 * dt(), 0, 0); */
        euler_rotation_mat4x4(&rot, 0.5, 0, 0);
        right_multiply_mat4x4(&sphere_matrix, &rot);
        /* euler_rotate_matrix4x4f(&sphere_matrix, -3 * dt(), 0, 0); */

        /* print_matrix4x4f(&sphere_matrix); */
    }
    if (arrow_key_down(Right)) {
        translate_matrix4x4f(&sphere_matrix, 1 * dt(), 0, 0);
        /* mat4x4 rot; */
        /* right_multiply_matrix(sphere_matrix */
    }

    /* if (alt_arrow_key_down */
    
    render_mesh(&basic_renderer, &sphere_mesh_handle, &sphere_matrix);
}
