/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

const int width = 100;
const int height = 100;
GLuint fb;
ResourceHandle texture_handle;
ResourceHandle material_handle;

const float polygon[] = {
    0.1,0,
    0.2,0.1,
    0.3,0.5,
    0.5,0.5,
    0.3,0.4,
};
const int n = sizeof(polygon) / (sizeof(float) * 2);

extern void input_event(int key, int action, int mods)
{
    static int n_pressed = 0;
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_P) {
            GLint prev_viewport[4];
            glGetIntegerv(GL_VIEWPORT, prev_viewport);
            glBindFramebuffer(GL_FRAMEBUFFER, fb);
            glViewport(0, 0, width, height);
            glClearColor(1,0,1,1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            int max = n < n_pressed ? n : n_pressed;
            printf("-\n");
            for (int i = 0; i < max; i++) {
                printf("drawing\n");
                int j = (i + 1) % max;
                //paint2d_triangle_m(0,0,  polygon[2*i],polygon[2*i+1],  polygon[2*j],polygon[2*j+1],  material_handle);
                paint2d_triangle_m(0,0,  0.5,0,  0,0.5,  material_handle);
                render_paint2d();
            }
            n_pressed ++;
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
	    glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
        }
    }
}
extern void cursor_move_event(double x, double y)
{
}
extern void init_program(void)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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

#if 0
    // create framebuffer
    Texture *texture = oneoff_resource(Texture, texture_handle);
    glGenTextures(1, &texture->texture_id);
    glBindTexture(GL_TEXTURE_2D, texture->texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
    glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->texture_id, 0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    GLuint depth_texture;
    glGenTextures(1, &depth_texture);
    glBindTexture(GL_TEXTURE_2D, depth_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("framebuffer incomplete\n");
        exit(EXIT_FAILURE);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif

    Texture *tex = oneoff_resource(Texture, texture_handle);
    tex->texture_id = texture;
    // load material
    material_handle = Material_create("Polygons/polygon");
    material_set_texture(resource_data(Material, material_handle), "tex", texture_handle);

    create_key_camera_man(0,0,0,  0,0,0);
}
extern void loop_program(void)
{
#if 1
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

    paint2d_sprite(0,0,  0.5,0.5,  texture_handle);
}
extern void close_program(void)
{
}
