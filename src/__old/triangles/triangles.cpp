
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ASCII_LINE "================================================================================\n"

#if 0
static const char *vertex_shader_text = 
"#version 110\n"
"uniform mat4 MVP;\n"
"attribute vec3 vCol;\n"
"attribute vec2 vPos;\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    color = vCol;\n"
"}\n";
#endif

#if 1
static const char *vertex_shader_text =
"#version 460\n"
"layout ( location = 0 ) in vec4 vPosition;\n"
"void\n"
"main()\n"
"{\n"
"   gl_Position = vPosition;\n"
"}\n";
#endif
static const char *fragment_shader_text =
"#version 460\n"
"out vec4 fColor;\n"
"void main()\n"
"{\n"
"   fColor = vec4(0.0, 0.0, 1.0, 1.0);\n"
"}\n";

// Global GL definitions
enum VAO_IDs { Triangles, NumVAOs };
enum Buffer_IDs { ArrayBuffer, NumBuffers };
enum Attrib_IDs { vPosition = 0 };

GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];

const GLuint NumVertices = 6;
//


GLuint LoadTriangleShaders(void)
{
    GLint is_good;
    GLuint vertex_shader, fragment_shader, program;
    
    /* GLuint glCreateShader(GLenum type);
     * -----------------------------------
     * Allocates a shader object. _type_ must be one of GL_VERTEX_SHADER, GL_TESS_CONTROL_SHADER,
     * GL_TESS_EVALUATION_SHADER, or GL_GEOMETRY_SHADER. The return value is either a nonzero
     * integer or zero if an error occured.
     */
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    /* void glShaderSource(GLuint shader, GLsizei count,
     *                     const GLchar **string, const GLint *length);
     * ----------------------------------------------------------------
     * Associates the source of a shader with a shader object _shader_.
     */
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &is_good);
    if (!is_good) {
        fprintf(stderr, "ERROR: Failed to compile vertex shader.\n");
        exit(EXIT_FAILURE);
    }


    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    // Count is 1 since the string array given is just a pointer to a single string (so char **).
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &is_good);
    if (!is_good) {
        fprintf(stderr, "ERROR: Failed to compile fragment shader.\n");
        exit(EXIT_FAILURE);
    }

    // Allocate program object handle
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &is_good);
    if (is_good == GL_FALSE) {
        fprintf(stderr, "ERROR: Failed to link program from shader objects.\n");
        exit(EXIT_FAILURE);
    }

    // ...

    return program;
}

void init_VAOs(void)
{
    /*
     */
    glGenVertexArrays(NumVAOs, VAOs);

    // Triangles
    glBindVertexArray(VAOs[Triangles]);
    GLfloat vertices[NumVertices][2] = {
        { -0.90, -0.90 },
        { 0.85, -0.90 },
        { -0.90, 0.85 },
        { 0.90, -0.85 },
        { 0.90, 0.90 },
        { -0.85, 0.90 }
    };
    glGenBuffers(NumBuffers, Buffers);
    glBindBuffer(GL_ARRAY_BUFFER, Buffers[ArrayBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, (void *) (0));
    glEnableVertexAttribArray(vPosition);
}


GLFWwindow *init_glfw_create_context(void)
{
    /* Attempts to create a GLFW OpenGL context associated to a window.
     * Errors are handled in this function, can just assume returned
     * GLFWwindow pointer is good.
     */
    if (!glfwInit()) {
        fprintf(stderr, "GLFW error: something went wrong initializing GLFW\n");
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow *window = glfwCreateWindow(512, 512, "Triangles", NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "GLFW error: failed to create a window properly\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    return window;
}


static void key(GLFWwindow *window, int key,
                int scancode, int action,
                int mods)
{
    if (action == GLFW_PRESS && (
                key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}


void display(void)
{
    // Draw the triangles
    glBindVertexArray(VAOs[Triangles]);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}


int main(int argc, char *argv[])
{
    printf("Starting triangles ...\n");

    // Context and window creation and setup
    GLFWwindow *window = init_glfw_create_context();
    gladLoadGL(); // now that a context can be active (?)
    glfwSwapInterval(1); // vsync and swapbuffers and stuff

    // Printout info
    printf("Vertex shader:\n");
    puts(vertex_shader_text);
    printf(ASCII_LINE);
    printf("Fragment shader:\n");
    puts(fragment_shader_text);
    printf(ASCII_LINE);

    // Create program object with shaders
    // This currently does not work ...
#if 0
    GLuint program;
    program = LoadTriangleShaders();
    glUseProgram(program);
#endif

    // Initialize data
    init_VAOs();

    // GLFW callback setup
    glfwSetKeyCallback(window, key);

    // Render and input GLFW loop
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        display();

        glfwSwapBuffers(window);
        glfwPollEvents();

        glFlush();
    }
    
    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
