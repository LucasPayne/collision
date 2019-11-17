/* PROJECT_LIBS:
 *      + glad
 *      + helper_gl
 *      + helper_input
 *      + entity
 *      + iterator
 *      - components/Transform2D
 * A polygon editor
 */

// How should this actually be done? Application-local files? Asset directory?
#define SHADERS_LOCATION "/home/lucas/code/collision/src/poly_editor/"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "helper_definitions.h"
#include "helper_gl.h"
#include "helper_input.h"
#include "entity.h"
#include "components/Transform2D.h"
#undef Transform2D_TYPE_ID
#define Transform2D_TYPE_ID 1



// Globals for testing -----------------------------------------------------------
GLuint triangle_vao;
DynamicShaderProgram dynamic_shader_program;
GLuint uniform_location_time;
//--------------------------------------------------------------------------------

static double ASPECT_RATIO;

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
    if (action == GLFW_PRESS && key == GLFW_KEY_C) {
        recompile_shader_program(&dynamic_shader_program);
    }
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{

}

void init_program(void)
{
    init_entity_model();
}
void loop(GLFWwindow *window)
{
    glUniform1f(uniform_location_time, time());

    glBindVertexArray(triangle_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    update_entity_model();
}
void close_program(void)
{
    close_entity_model();
}

void reshape(GLFWwindow* window, int width, int height)
{
    force_aspect_ratio(window, width, height, ASPECT_RATIO);
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    /* glScissor(viewport[0], viewport[1], viewport[2], viewport[3]); */
    printf("x:%d, y: %d, width: %d, height: %d\n", viewport[0], viewport[1], viewport[2], viewport[3]);
}

int main(int argc, char *argv[])
{
    GLFWwindow *window;
    int horiz = 512;
    int vert = 512;
    char *name = "Polygon editor";
    //--------------------------------------------------------------------------------
    if (!glfwInit()) {
        fprintf(stderr, "GLFW error: something went wrong initializing GLFW\n");
        exit(EXIT_FAILURE);
    }

    /* glmeta
     * Use a VBO to render a single, 2D flat white triangle to the screen. OpenGL with the compatibility profile
     * will render this without needing shaders or matrix operations. It will fallback to fixed function for the rendering
     * and provide you with a viewport with -1.0 to 1.0 for the x & y coordinates. Z coordinates will be ignored.
     * ---
     * No default pass-through white vertex and fragment shaders (""?) with the core profile.
     */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window = glfwCreateWindow(horiz, vert, name, NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "GLFW error: failed to create a window properly\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);

    gladLoadGL();
    glfwSwapInterval(1);

    // Vertex attributes -------------------------------------------------------------
    const int vPosition = 0;
    const int vColor = 1;
    //--------------------------------------------------------------------------------
    // Shaders
    //--------------------------------------------------------------------------------
    char *vertex_shader_path = SHADERS_LOCATION "shader.vert";
    char *fragment_shader_path = SHADERS_LOCATION "shader.frag";
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    
    load_and_compile_shader(vertex_shader, vertex_shader_path);
    load_and_compile_shader(fragment_shader, fragment_shader_path);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
        // Attachments
        glBindAttribLocation(shader_program, vPosition, "vPosition");
        glBindAttribLocation(shader_program, vColor, "vColor");
    link_shader_program(shader_program);

    /* glDeleteShader(vertex_shader); */
    /* glDeleteShader(fragment_shader); */

    glUseProgram(shader_program);

    // Initialize dynamic shader program for run-time recompilation
    // global DynamicShaderProgram dynamic_shader_program;
    dynamic_shader_program.id = shader_program;
    dynamic_shader_program.vertex_shader_id = vertex_shader;
    dynamic_shader_program.fragment_shader_id = fragment_shader;
    if (strlen(vertex_shader_path) > MAX_DYNAMIC_SHADER_PROGRAM_PATH_LENGTH ||
            strlen(fragment_shader_path) > MAX_DYNAMIC_SHADER_PROGRAM_PATH_LENGTH) {
        fprintf(stderr, "ERROR: path name for shader too long.\n");
        exit(EXIT_FAILURE);
    }
    strcpy(dynamic_shader_program.vertex_shader_path, vertex_shader_path);
    strcpy(dynamic_shader_program.fragment_shader_path, fragment_shader_path);

    print_dynamic_shader_program(&dynamic_shader_program);

    //--------------------------------------------------------------------------------
    // Vertex data and specification -------------------------------------------------

    float triangle_data[3 * 2] = {
        -0.5, 0.5,
        -0.5, -0.5,
        0.5, -0.5
    };
    float triangle_color_data[3 * 4] = {
        1.0, 0.0, 0.0, 1.0,
        0.0, 1.0, 0.0, 1.0,
        0.0, 0.0, 1.0, 1.0
    };

    // global GLuint triangle_vao;
    glGenVertexArrays(1, &triangle_vao);
    glBindVertexArray(triangle_vao);

    GLuint triangle_vbo;
    glGenBuffers(1, &triangle_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_data) + sizeof(triangle_color_data), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(triangle_data), triangle_data);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(triangle_data), sizeof(triangle_color_data), triangle_color_data);

    glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, (void *) sizeof(triangle_data));
    glEnableVertexAttribArray(vColor);

    // Uniforms ----------------------------------------------------------------------
    // global GLuint uniform_location_time;
    // ---- MUST update on relink
    uniform_location_time = glGetUniformLocation(shader_program, "time");

    //--------------------------------------------------------------------------------
    ASPECT_RATIO = SCREEN_ASPECT_RATIO;
    glEnable(GL_SCISSOR_TEST);

    glClearColor(1.0, 1.0, 1.0, 1.0);

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetFramebufferSizeCallback(window, reshape);

    init_program();
    loop_time(window, loop, GL_COLOR_BUFFER_BIT);
    close_program();

    glfwTerminate();

    exit(EXIT_SUCCESS);
}
