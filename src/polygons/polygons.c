/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

const int width = 64;
const int height = 64;
GLuint fb;
ResourceHandle texture_handle;
// ResourceHandle material_handle;

#define max_points 1024
static int n = 0;
static float polygon[max_points * 2];

bool S_demo = false;

void add_point(float x, float y)
{
    printf("Adding point at (%.2f, %.2f)\n", x, y);
    polygon[2*n] = x;
    polygon[2*n+1] = y;
    n ++;
}

extern void input_event(int key, int action, int mods)
{
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_S) {
            S_demo = true;
        }
    }
}
extern void cursor_move_event(double x, double y)
{
}
extern void mouse_button_event(int button, int action, int mods)
{
    if (action == GLFW_PRESS) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            vec2 pos = pixel_to_rect(mouse_x,mouse_y,  0,0,  1,1);
            add_point(pos.vals[0], pos.vals[1]);
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
    //---painting system is destroying resource handles currently, so not doing this.
    // material_handle = Material_create("Polygons/polygon");
    // material_set_texture(resource_data(Material, material_handle), "tex", texture_handle);

    create_key_camera_man(0,0,0,  0,0,0);
}

extern void loop_program(void)
{
    #define between 0.14
    #define frames 20
    static float timer = between;
    static float S_frame = 0;
    if (S_demo) {
        timer -= dt;
        if (timer < 0) {
            float y;
            if (S_frame >= frames) y = 0.2 + 0.6 * (1 - (S_frame - frames) / frames) + 0.1;
            else y = 0.2 + 0.6 * S_frame / frames;
            float x;
            if (S_frame >= frames) x = 0.5 + 0.3 * sin(-2 * M_PI * S_frame / frames);
            else x = 0.5 + 0.3 * sin(2 * M_PI * S_frame / frames);
            add_point(x, y);
            timer = between;
            S_frame ++;
            if (S_frame > frames * 2) S_demo = false;
        }
    }

    GLint prev_viewport[4];
    glGetIntegerv(GL_VIEWPORT, prev_viewport);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glViewport(0, 0, width, height);
    glClearColor(0,0,1,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (n > 2) {
        for (int i = 0; i < n; i++) {
            int j = (i + 1) % n;
            //paint2d_triangle_m(0,0,  polygon[2*i],polygon[2*i+1],  polygon[2*j],polygon[2*j+1],  Material_create("Materials/red"));
            ResourceHandle material_handle = Material_create("Polygons/polygon");
            material_set_texture(resource_data(Material, material_handle), "tex", texture_handle);
            paint2d_triangle_m(0,0,  polygon[2*i],polygon[2*i+1],  polygon[2*j],polygon[2*j+1],  material_handle);
        }
    }
    render_paint2d();
    painting_flush(Canvas2D);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);

    paint2d_sprite(0,0,  1,1,  texture_handle);
}
extern void close_program(void)
{
}
