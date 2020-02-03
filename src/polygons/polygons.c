/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

const int width = 32;
const int height = 32;
GLuint fb;
ResourceHandle texture_handle;
ResourceHandle material_handle;

#define max_points 1024
static int n = 0;
static float polygon[max_points];

void add_point(float x, float y)
{
    printf("Adding point at (%.2f, %.2f)\n", x, y);
    polygon[2*n] = x * 0.01;
    polygon[2*n+1] = y * 0.01;
    n ++;

    GLint prev_viewport[4];
    glGetIntegerv(GL_VIEWPORT, prev_viewport);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glViewport(0, 0, width, height);
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    printf("-\n");
    for (int i = 0; i < n; i++) {
        printf("drawing\n");
        int j = (i + 1) % n;
        paint2d_triangle_m(0,0,  polygon[2*i],polygon[2*i+1],  polygon[2*j],polygon[2*j+1],  material_handle);
        render_paint2d();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
}

extern void input_event(int key, int action, int mods)
{
}
extern void cursor_move_event(double x, double y)
{
}
extern void mouse_button_event(int button, int action, int mods)
{
    if (action == GLFW_PRESS) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            add_point(mouse_x, mouse_y);
        }
    }
}

extern void init_program(void)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
    // Framebuffer
    glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, ERROR_ALERT "framebuffer incomplete");
        exit(EXIT_FAILURE);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Texture *tex = oneoff_resource(Texture, texture_handle);
    tex->texture_id = texture;
    // load material
    material_handle = Material_create("Polygons/polygon");
    material_set_texture(resource_data(Material, material_handle), "tex", texture_handle);

    create_key_camera_man(0,0,0,  0,0,0);
}
extern void loop_program(void)
{
#if 0
    GLint prev_viewport[4];
    glGetIntegerv(GL_VIEWPORT, prev_viewport);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glViewport(0, 0, width, height);
    glClearColor(1,0,1,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //paint2d_triangle_m(0,0,  0.5,0,  0,0.5,  material_handle);
    // paint2d_rect_c(0,0,  1,1,  "r");
    // render_paint2d();
    // painting_flush(Canvas2D);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
#endif
    // printf("%d\n", resource_data(Texture, texture_handle)->texture_id);

    paint2d_sprite(0,0,  1,1,  texture_handle);
}
extern void close_program(void)
{
}
