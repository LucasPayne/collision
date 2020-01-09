#ifndef HEADER_DEFINED_SPHERE
#define HEADER_DEFINED_SPHERE
static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void init_program(void);
void loop(GLFWwindow *window);
void close_program(void);
void reshape(GLFWwindow* window, int width, int height);
static void key_callback(GLFWwindow *window, int key,
                int scancode, int action,
                int mods);
#endif // HEADER_DEFINED_SPHERE
