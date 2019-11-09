/*
 *
 */

#ifndef HEADER_DEFINED_HELPER_INPUT
#define HEADER_DEFINED_HELPER_INPUT

#include <GLFW/glfw3.h>
#include <stdbool.h>

/* typedef void (*)(GLFWwindow *, int, int, int, int) KeyCallbackFunction; */

enum Dir {
	Up, Down, Left, Right,
        NumDirs
};

bool is_down(int key);
bool alt_is_down(int key);
bool is_up(int key);
bool alt_is_up(int key);
bool is_left(int key);
bool alt_is_left(int key);
bool is_right(int key);
bool alt_is_right(int key);
bool is_arrow_key(int key);
bool alt_is_arrow_key(int key);
bool key_is_dir(int dir, int key);
bool alt_key_is_dir(int dir, int key);

bool arrow_key_down(int key);
bool alt_arrow_key_down(int key);
void set_arrow_key_down(int key);
void alt_set_arrow_key_down(int key);
void set_arrow_key_up(int key);
void alt_set_arrow_key_up(int key);
void key_callback_arrows_down(GLFWwindow *window, int key,
                int scancode, int action,
                int mods);
void key_callback_quit(GLFWwindow *window, int key,
                int scancode, int action,
                int mods);

#endif
