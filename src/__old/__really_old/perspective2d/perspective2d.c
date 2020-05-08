/* PROJECT_LIBS:
 *      + glad
 *      + helper_gl
 *      + helper_input
 *      + matrix_mathematics
 * 
 * CREATED FROM SHADERS SCHEMATIC
 */
#define SHADERS_LOCATION "/home/lucas/code/collision/src/perspective2d/"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "helper_gl.h"
#include "helper_input.h"
#include "matrix_mathematics.h"

// Globals for testing -----------------------------------------------------------
DynamicShaderProgram dynamic_shader_program;
static double ASPECT_RATIO;

static mat3x3 model_matrix;
static mat3x3 projection_matrix;

static mat4x4 camera_matrix;

static GLuint uniform_location_model_matrix;
static GLuint uniform_location_projection_matrix;
static GLuint uniform_location_aspect_ratio;
static GLuint uniform_location_camera_matrix;
static GLuint shader_program;

static GLuint polygon_vao;
static GLuint floor_vao;

static float poly_theta;
static float poly_x;
static float poly_y;

static float camera_matrix_x = 0.0;
static float camera_matrix_y = 0.0;
static float camera_matrix_z = 0.0;
static float camera_matrix_theta_x = 0.0;
static float camera_matrix_theta_y = 0.0;
static float camera_matrix_theta_z = 0.0;

//--------------------------------------------------------------------------------

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
    identity_matrix3x3f(&model_matrix);
    identity_matrix3x3f(&projection_matrix);

    identity_matrix4x4f(&camera_matrix);

    uniform_location_model_matrix = glGetUniformLocation(shader_program, "model_matrix");
    uniform_location_aspect_ratio = glGetUniformLocation(shader_program, "aspect_ratio");
    uniform_location_projection_matrix = glGetUniformLocation(shader_program, "projection_matrix");
    uniform_location_camera_matrix = glGetUniformLocation(shader_program, "camera_matrix");

    {
        float vertices[2 * 3] = {
            0.0, 0.0,
            1.0, 0.0,
            2.0, 1.0,
            /* 0.0, 2.0, */
            /* -1.0, 0.5 */
        };
        GLuint vertex_buffer;
        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

        const GLuint attribute_location_vPosition = 0;
        /* global GLuint polygon_vao; */
        glGenVertexArrays(1, &polygon_vao);
        glBindVertexArray(polygon_vao);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glVertexAttribPointer(attribute_location_vPosition, 2, GL_FLOAT, GL_FALSE, 0, (void *) 0);
        glEnableVertexAttribArray(attribute_location_vPosition);

    }
    /* { */
    /*     float vertices[ */
    /*     /1* global GLuint floor_vao; *1/ */
    /*     glGenVertexArrays(1, &floor_vao); */
    /*     glBindVertexArray(floor_vao); */
    /*     glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer */
    /* } */

    poly_x = 0.0;
    poly_y = 0.0;
    poly_theta = 0.0;
}

static float projection_theta_z = 0.0;
void loop(GLFWwindow *window)
{
    glUniform1f(uniform_location_aspect_ratio, ASPECT_RATIO);

    // controls
    if (arrow_key_down(Left))
        poly_x -= 3 * dt();
    if (arrow_key_down(Right))
        poly_x += 3 * dt();

    if (arrow_key_down(Up))
        projection_theta_z += 1.0 * dt();
    if (arrow_key_down(Down))
        projection_theta_z -= 1.0 * dt();

    if (alt_arrow_key_down(Down))
        camera_matrix_z -= 1.0 * dt();
    if (alt_arrow_key_down(Up))
        camera_matrix_z += 1.0 * dt();
    if (alt_arrow_key_down(Left))
        camera_matrix_x -= 1.0 * dt();
    if (alt_arrow_key_down(Right))
        camera_matrix_x += 1.0 * dt();

    euler_rotation_mat3x3(&projection_matrix, 0, 0, projection_theta_z);

    translate_rotate_3d_mat4x4(&camera_matrix, camera_matrix_x, camera_matrix_y, camera_matrix_z, camera_matrix_theta_x, camera_matrix_theta_y, camera_matrix_theta_z);

    // Automatic upsize? 0 0 0 1 translating column, 0 0 0 1 bottom row
    /* mat4x4 upsized_model_matrix; */
    /* for (int i = 0; i < 3; i++) { */
    /*     for (int j = 0; j < 3; j++) { */
    /*         upsized_model_matrix.vals[i + 4 * j] = model_matrix.vals[i + 3 * j]; */
    /*     } */
    /* } */
    /* for (int i = 0; i < 3; i++) { */
    /*     upsized_model_matrix.vals[3 + 4 * i] = 0; */
    /*     upsized_model_matrix.vals[i + 4 * 3] = 0; */
    /* } */
    /* upsized_model_matrix.vals[3 + 4*3] = 1; */
    // is there a non v version?

    translate_rotate_2d_matrix3x3f(&model_matrix, poly_x, poly_y, poly_theta);
    /* poly_theta += dt(); */

    glUniformMatrix3fv(uniform_location_projection_matrix, 1, GL_TRUE, (const GLfloat *) &projection_matrix.vals);
    glUniformMatrix3fv(uniform_location_model_matrix, 1, GL_TRUE, (const GLfloat *) &model_matrix.vals);
    glUniformMatrix4fv(uniform_location_camera_matrix, 1, GL_TRUE, (const GLfloat *) &camera_matrix.vals);
    glBindVertexArray(polygon_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    /* print_matrix3x3f(&model_matrix); */
    /* printf("---\n"); */
}


void close_program(void)
{
}


void reshape(GLFWwindow* window, int width, int height)
{
    force_aspect_ratio(window, width, height, ASPECT_RATIO);
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    printf("x:%d, y: %d, width: %d, height: %d\n", viewport[0], viewport[1], viewport[2], viewport[3]);
}


int main(int argc, char *argv[])
{
    GLFWwindow *window;
    int horiz = 512;
    int vert = 512;
    char *name = "Shader testing";
    //--------------------------------------------------------------------------------
    if (!glfwInit()) {
        fprintf(stderr, "GLFW error: something went wrong initializing GLFW\n");
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(horiz, vert, name, NULL, NULL);
    if (!window) {
        fprintf(stderr, "GLFW error: failed to create a window properly.\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glfwSwapInterval(1);

    // Shaders
    //--------------------------------------------------------------------------------
    char *vertex_shader_path = SHADERS_LOCATION "2dperspective.vert";
    char *fragment_shader_path = SHADERS_LOCATION "2dperspective.frag";
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    
    load_and_compile_shader(vertex_shader, vertex_shader_path);
    load_and_compile_shader(fragment_shader, fragment_shader_path);

    /* global GLuint */ shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    link_shader_program(shader_program);
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

    ASPECT_RATIO = SCREEN_ASPECT_RATIO;
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetFramebufferSizeCallback(window, reshape);

    init_program();
    loop_time(window, loop, GL_COLOR_BUFFER_BIT);
    close_program();

    glfwTerminate();

    exit(EXIT_SUCCESS);
}
