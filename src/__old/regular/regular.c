#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "helper_gl.h"
#include "helper_input.h"

#define PI 3.1415926

#define ASCII_LINE "================================================================================\n"

double TIME;
double DT;
int REGULAR_POLYGON_VERTICES = 4;
double POLYGON_HORIZ_POS = 0.0;

static void key_callback(GLFWwindow *window, int key,
                int scancode, int action,
                int mods)
{
    if (action == GLFW_PRESS)  {
        if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
    }
    else if  (action == GLFW_RELEASE) {
        //
    }

    key_callback_arrows_down(window, key, scancode, action, mods);
}

void display(void)
{
    glBegin(GL_POLYGON);
    
    int n = REGULAR_POLYGON_VERTICES;
    for (int i = 0; i < n; i++) {
        glVertex2f( cos(TIME + i * 2*PI/n) + 0.1 * sin(TIME * 2.0)
                        + POLYGON_HORIZ_POS
                , sin(TIME + i * 2*PI/n) + 0.3 * cos(TIME * 2.111) );
    }
    /* glVertex2f( 0, 0 ); */
    /* glVertex2f( 0.5, 0 ); */
    /* glVertex2f( 0.5, 0.5 ); */
    /* glVertex2f( 0, 0.5 ); */
    glEnd();
}


int main(int argc, char *argv[])
{
    printf("Starting grid ...\n");

    // Context and window creation and setup
    GLFWwindow *window = init_glfw_create_context("Regular polygon", 512, 512);
    gladLoadGL(); // now that a context can be active (?)
    glfwSwapInterval(1); // vsync and swapbuffers and stuff

    // Create program object with shaders
    // This currently does not work ...
#if 0
    GLuint program;
    program = LoadShaders();
    glUseProgram(program);
#endif

    // GLFW callback setup
    glfwSetKeyCallback(window, key_callback);

    // Render and input GLFW loop
    TIME = glfwGetTime();
    DT = 0.0;
    double last_time = TIME;
    while (!glfwWindowShouldClose(window))
    {
        last_time = TIME;
        TIME = glfwGetTime();
        DT = TIME - last_time;

	// Input handling
	if (arrow_key_down(Left)) { POLYGON_HORIZ_POS -= DT * 1.0; }
	if (arrow_key_down(Right)) { POLYGON_HORIZ_POS += DT * 1.0; }

        if (arrow_key_down(Up)) { REGULAR_POLYGON_VERTICES ++; }
        if (REGULAR_POLYGON_VERTICES > 3 && arrow_key_down(Down)) { REGULAR_POLYGON_VERTICES --; }

	// Display
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
