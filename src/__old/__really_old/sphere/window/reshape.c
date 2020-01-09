
void reshape(GLFWwindow* window, int width, int height)
{
    force_aspect_ratio(window, width, height, ASPECT_RATIO);
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    printf("x:%d, y: %d, width: %d, height: %d\n", viewport[0], viewport[1], viewport[2], viewport[3]);
}

