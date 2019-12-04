/* PROJECT_LIBS:
 *      + glad
 *      + helper_gl
 *      + helper_input
 *      + matrix_mathematics
 *      + mesh_gen
 */
#define SHADERS_LOCATION "/home/lucas/code/collision/src/cube/"

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
#include "mesh_gen.h"

// Globals for testing -----------------------------------------------------------
DynamicShaderProgram dynamic_shader_program;
static double ASPECT_RATIO;

static GLuint uniform_location_time;
static GLuint uniform_location_aspect_ratio;
static GLuint uniform_location_rotation_matrix;
static GLuint uniform_location_view_matrix;

static GLuint cube_triangles_vao;
static GLuint cube_vertices_vao;

#define CONTROL_MODE_CUBE 0
#define CONTROL_MODE_CAMERA 1
static int control_mode = 0;

static float theta_x = 0.0;
static float theta_y = 0.0;
static float theta_z = 0.0;
static float cube_x = 0.0;
static float cube_y = 0.0;
static float cube_z = 0.0;

typedef struct Transform_s {
    float theta_x;
    float theta_y;
    float theta_z;
    float x;
    float y;
    float z;
} Transform;
void transform_to_matrix(Transform *transform, Matrix4x4f *matrix)
{
    euler_rotation_matrix4x4f(matrix, transform->theta_x, transform->theta_y, transform->theta_z);
    translate_matrix4x4f(matrix, transform->x, transform->y, transform->z);
}

static Transform camera_transform;

//--------------------------------------------------------------------------------

static void key_callback(GLFWwindow *window, int key,
                int scancode, int action,
                int mods)
{
    key_callback_quit(window, key, scancode, action, mods);
    key_callback_arrows_down(window, key, scancode, action, mods);

    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_SPACE) {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            printf("%d %d\n", width, height);
        }
        else if (key == GLFW_KEY_C) {
            recompile_shader_program(&dynamic_shader_program);
        }
        else if (key == GLFW_KEY_R) {
            if (control_mode == CONTROL_MODE_CUBE)
                control_mode = CONTROL_MODE_CAMERA;
            else if (control_mode == CONTROL_MODE_CAMERA)
                control_mode = CONTROL_MODE_CUBE;
        }
    }
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{

}

void init_program(void)
{
    // Texturing
    float texture_data[4*4 * 4] = {
        0, 0, 1, 1,   0, 1, 0, 1,   0, 0, 0, 1,   1, 1, 0, 1,
        1, 0, 1, 1,   0, 1, 0, 1,   0, 1, 0, 1,   1, 1, 0, 1,
        0, 0, 1, 1,   0, 0, 0, 1,   0, 0, 0, 1,   0, 1, 0, 1,
        0, 0, 1, 1,   0, 1, 1, 1,   0, 1, 0, 1,   1, 1, 0, 1,
    };
    /* // Black white checkerboard */
    /* float texture_data[4*4 * 4] = { */
    /*     1, 1, 1, 1,   0, 0, 0, 1,   1, 1, 1, 1,   0, 0, 0, 1, */
    /*     0, 0, 0, 1,   1, 1, 1, 1,   0, 0, 0, 1,   1, 1, 1, 1, */
    /*     1, 1, 1, 1,   0, 0, 0, 1,   1, 1, 1, 1,   0, 0, 0, 1, */
    /*     0, 0, 0, 1,   1, 1, 1, 1,   0, 0, 0, 1,   1, 1, 1, 1, */
    /* }; */
    GLuint texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB16, 4, 4);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 4, GL_RGBA, GL_FLOAT, texture_data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Vertex array object initialization
    const GLuint vPosition = 0;
    const GLuint vColor = 1;
    const GLuint texture_coord = 2;

    //--------------------------------------------------------------------------------
    GLuint cube_colors;
    glGenBuffers(1, &cube_colors);
    glBindBuffer(GL_ARRAY_BUFFER, cube_colors);
    float colors[4 * 8] = {
        1.0, 1.0, 1.0, 1.0,
        0.0, 1.0, 1.0, 1.0,
        1.0, 0.0, 1.0, 1.0,
        1.0, 1.0, 0.0, 1.0,
        0.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 0.0, 1.0,
        0.0, 1.0, 0.0, 1.0,
        0.0, 0.0, 1.0, 1.0,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_DYNAMIC_DRAW);
    //--------------------------------------------------------------------------------
    Mesh mesh;
    create_cube_mesh(&mesh, 0.7);
    //--------------------------------------------------------------------------------
    GLuint cube_vertices;
    glGenBuffers(1, &cube_vertices);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vertices);
    glBufferData(GL_ARRAY_BUFFER, mesh.num_vertices * 3*sizeof(float), mesh.vertices, GL_DYNAMIC_DRAW);
    //--------------------------------------------------------------------------------
    GLuint cube_triangle_indices;
    glGenBuffers(1, &cube_triangle_indices);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vertices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_triangle_indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.num_triangles * 3*sizeof(int), mesh.triangles, GL_DYNAMIC_DRAW);
    //--------------------------------------------------------------------------------
    free_mesh(&mesh);
    // Texture mapping
    float texture_coords_data[8*2] = {
        -1, -1,
        1, -1,
        1, 1,
        -1, 1,

        1, -1,
        1, 1,
        -1, 1,
        -1, -1,
    };
    GLuint texture_coords;
    glGenBuffers(1, &texture_coords);
    glBindBuffer(GL_ARRAY_BUFFER, texture_coords);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texture_coords_data), texture_coords_data, GL_DYNAMIC_DRAW);
    
    // Create vertex array objects and set vertex attributes

    // Cube triangles VAO. Indexed triangle list (array elements).
    glGenVertexArrays(1, &cube_triangles_vao);
    glBindVertexArray(cube_triangles_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_triangle_indices);
    // vPosition attribute
    glBindBuffer(GL_ARRAY_BUFFER, cube_vertices);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    glEnableVertexAttribArray(vPosition);
    // vColor attribute
    glBindBuffer(GL_ARRAY_BUFFER, cube_colors);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    glEnableVertexAttribArray(vColor);
    // texture_coord attribute
    glBindBuffer(GL_ARRAY_BUFFER, texture_coords);
    glVertexAttribPointer(texture_coord, 2, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    glEnableVertexAttribArray(texture_coord);

    // Cube vertices VAO. Non-indexed, used for point-drawing.
    glGenVertexArrays(1, &cube_vertices_vao);
    glBindVertexArray(cube_vertices_vao);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vertices);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    glEnableVertexAttribArray(vPosition);
    glBindBuffer(GL_ARRAY_BUFFER, cube_colors);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    glEnableVertexAttribArray(vColor);

    // Depth testing
    /* glEnable(GL_DEPTH_TEST); */

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    printf("Maximum number of vertex attributes: %d\n", GL_MAX_VERTEX_ATTRIBS);

    // Cube rotating
    theta_x = 0.0;
    theta_y = 0.0;
    theta_z = 0.0;
    cube_x = 0.0;
    cube_y = 0.0;
    cube_z = 0.0;

    // Camera
    memset(&camera_transform, 0, sizeof(Transform)); // ...
}

void draw_cube(Matrix4x4f *matrix)
{
    glUniformMatrix4fv(uniform_location_rotation_matrix, 1, GL_TRUE, (const GLfloat *) &matrix->vals);
#if 1
    glBindVertexArray(cube_vertices_vao);
    glPointSize(10);
    glDrawArrays(GL_POINTS, 0, 8);
#endif
    glBindVertexArray(cube_triangles_vao);
    /* glDrawElements(GL_LINES, 3 * 12, GL_UNSIGNED_INT, (void *) 0); */
    glDrawElements(GL_TRIANGLES, 3 * 12, GL_UNSIGNED_INT, (void *) 0);
}


static int FRAME = 0;
void loop(GLFWwindow *window)
{
    // controls
    if (control_mode == CONTROL_MODE_CUBE) {
        float rot_speed = 3.0;
        if (arrow_key_down(Down)) {
            theta_x += rot_speed * dt();
        }
        if (arrow_key_down(Up)) {
            theta_x -= rot_speed * dt();
        }
        if (arrow_key_down(Left)) {
            theta_z += rot_speed * dt();
        }
        if (arrow_key_down(Right)) {
            theta_z -= rot_speed * dt();
        }
        float translate_speed = 1.5;
        if (alt_arrow_key_down(Up)) {
            cube_z += translate_speed * dt();
        }
        if (alt_arrow_key_down(Down)) {
            cube_z -= translate_speed * dt();
        }
        if (alt_arrow_key_down(Left)) {
            cube_x -= translate_speed * dt();
        }
        if (alt_arrow_key_down(Right)) {
            cube_x += translate_speed * dt();
        }
    }
    else if (control_mode == CONTROL_MODE_CAMERA) {
        float rot_speed = 3.0;
        if (arrow_key_down(Down)) {
            camera_transform.theta_x += rot_speed * dt();
        }
        if (arrow_key_down(Up)) {
            camera_transform.theta_x -= rot_speed * dt();
        }
        if (arrow_key_down(Left)) {
            camera_transform.theta_y += rot_speed * dt();
        }
        if (arrow_key_down(Right)) {
            camera_transform.theta_y -= rot_speed * dt();
        }
        float translate_speed = 10;
        if (alt_arrow_key_down(Up)) {
            camera_transform.z += translate_speed * dt();
        }
        if (alt_arrow_key_down(Down)) {
            camera_transform.z -= translate_speed * dt();
        }
        if (alt_arrow_key_down(Left)) {
            camera_transform.x -= translate_speed * dt();
        }
        if (alt_arrow_key_down(Right)) {
            camera_transform.x += translate_speed * dt();
        }
    }

    glUniform1f(uniform_location_time, time());
    glUniform1f(uniform_location_aspect_ratio, ASPECT_RATIO);
    Matrix4x4f view_matrix;
    transform_to_matrix(&camera_transform, &view_matrix);
    /* printf("view matrix:\n"); */
    /* print_matrix4x4f(&view_matrix); */

    glUniformMatrix4fv(uniform_location_view_matrix, 1, GL_TRUE, (const GLfloat *) &view_matrix.vals);
    Matrix4x4f rotation_matrix;
    euler_rotation_matrix4x4f(&rotation_matrix, theta_x, theta_y, theta_z);
    /* printf("Rotation matrix:\n"); */
    /* print_matrix4x4f(&rotation_matrix); */
    translate_matrix4x4f(&rotation_matrix, cube_x, cube_y, cube_z);
    /* printf("Rotation and translation matrix:\n"); */
    /* print_matrix4x4f(&rotation_matrix); */

    /* glDepthFunc(GL_LESS); */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            Matrix4x4f rot;
            copy_matrix4x4f(&rot, &rotation_matrix);
            translate_matrix4x4f(&rot, i, j, 0);
            Matrix4x4f extra_rot;
            euler_rotation_matrix4x4f(&extra_rot, i * 0.6, j * 0.6, (i + j) * 0.3);
            right_multiply_matrix4x4f(&rot, &extra_rot);
            draw_cube(&rot);
        }
    }

    /* glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void *) (3 * sizeof(float) * (FRAME/5 % 12))); */

    FRAME ++;
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
    const char *vertex_shader_path = SHADERS_LOCATION "shader.vert";
    const char *fragment_shader_path = SHADERS_LOCATION "shader.frag";
    const char *geometry_shader_path = SHADERS_LOCATION "shader.geom";
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    /* GLuint geometry_shader = glCreateShader(GL_GEOMETRY_SHADER); */
    
    load_and_compile_shader(vertex_shader, vertex_shader_path);
    load_and_compile_shader(fragment_shader, fragment_shader_path);
    /* load_and_compile_shader(geometry_shader, geometry_shader_path); */

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    /* glAttachShader(shader_program, geometry_shader); */
    link_shader_program(shader_program);
    glUseProgram(shader_program);

    // global uniforms
    uniform_location_time = glGetUniformLocation(shader_program, "time");
    uniform_location_aspect_ratio = glGetUniformLocation(shader_program, "aspect_ratio");
    uniform_location_rotation_matrix = glGetUniformLocation(shader_program, "rotation_matrix");
    uniform_location_view_matrix = glGetUniformLocation(shader_program, "view_matrix");

    // Initialize dynamic shader program for run-time recompilation
    // global DynamicShaderProgram dynamic_shader_program;
    dynamic_shader_program.id = shader_program;
    dynamic_shader_program.vertex_shader_id = vertex_shader;
    dynamic_shader_program.fragment_shader_id = fragment_shader;
    /* dynamic_shader_program.geometry_shader_id = geometry_shader; */
    dynamic_shader_program.geometry_shader_id = NO_SHADER;
    strcpy(dynamic_shader_program.vertex_shader_path, vertex_shader_path);
    strcpy(dynamic_shader_program.fragment_shader_path, fragment_shader_path);
    /* strcpy(dynamic_shader_program.geometry_shader_path, geometry_shader_path); */
    print_dynamic_shader_program(&dynamic_shader_program);

    ASPECT_RATIO = SCREEN_ASPECT_RATIO;
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClearDepth(1.0);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetFramebufferSizeCallback(window, reshape);

    init_program();
    loop_time(window, loop, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    close_program();

    glfwTerminate();

    exit(EXIT_SUCCESS);
}
