
static void key_callback(GLFWwindow *window, int key,
                int scancode, int action,
                int mods)
{
    key_callback_quit(window, key, scancode, action, mods);
    key_callback_arrows_down(window, key, scancode, action, mods);

    if (action == GLFW_PRESS && key == GLFW_KEY_SPACE) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        printf("%d %d\n", width, height);
    }
    /* if (action == GLFW_PRESS && key == GLFW_KEY_C) { */
    /*     recompile_shader_program(&dynamic_shader_program); */
    /* } */

    if (action == GLFW_PRESS && key == GLFW_KEY_R) {
        print_renderer(&basic_renderer);
    }
}

