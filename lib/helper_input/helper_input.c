/*--------------------------------------------------------------------------------
    Definitions for the input helper module.
--------------------------------------------------------------------------------*/
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include "helper_input.h"

// Must be in the same order as the SpecialKeys enum is declared.
static GLint GLFW_SPECIAL_KEY_MAP[NUM_SPECIAL_KEYS] = {
    GLFW_KEY_LEFT_SHIFT,
    GLFW_KEY_RIGHT_SHIFT,
    GLFW_KEY_SPACE,
    GLFW_KEY_ENTER,
    GLFW_KEY_TAB,
};
static bool SPECIAL_KEY_DOWN[NUM_SPECIAL_KEYS];

// Dual hjkl and arrow keys
bool is_down(int key)
{   // DOWN
    return key == GLFW_KEY_DOWN || key == GLFW_KEY_J;
}
bool alt_is_down(int key) 
{   // ALT DOWN
    return key == GLFW_KEY_S;
}
bool is_up(int key) 
{   // UP
    return key == GLFW_KEY_UP || key == GLFW_KEY_K;
}
bool alt_is_up(int key) 
{   // ALT UP
    return key == GLFW_KEY_W;
}
bool is_left(int key) 
{   // LEFT
    return key == GLFW_KEY_LEFT || key == GLFW_KEY_H;
}
bool alt_is_left(int key) 
{   // ALT LEFT
    return key == GLFW_KEY_A; 
}
bool is_right(int key) 
{   // RIGHT
    return key == GLFW_KEY_RIGHT || key == GLFW_KEY_L;
}
bool alt_is_right(int key) 
{   // ALT RIGHT
    return key == GLFW_KEY_D;
}
bool is_arrow_key(int key)
{
    return is_down(key) || is_up(key) || is_left(key) || is_right(key);
}
bool alt_is_arrow_key(int key)
{
    return alt_is_down(key) || alt_is_up(key) || alt_is_left(key) || alt_is_right(key);
}

bool key_is_dir(int dir, int key)
{
    if (dir == Up) return is_up(key);
    if (dir == Down) return is_down(key);
    if (dir == Left) return is_left(key);
    if (dir == Right) return is_right(key);
    return false;
}
bool alt_key_is_dir(int dir, int key)
{
    if (dir == Up) return alt_is_up(key);
    if (dir == Down) return alt_is_down(key);
    if (dir == Left) return alt_is_left(key);
    if (dir == Right) return alt_is_right(key);
    return false;
}
static bool _ARROW_KEY_DOWN[4] = {false};
static bool _ALT_ARROW_KEY_DOWN[4] = {false};
bool arrow_key_down(int key)
{
    return _ARROW_KEY_DOWN[key];
}
void set_arrow_key_down(int key)
{
    _ARROW_KEY_DOWN[key] = true;
}
void set_arrow_key_up(int key)
{
    _ARROW_KEY_DOWN[key] = false;
}
bool alt_arrow_key_down(int key)
{
    return _ALT_ARROW_KEY_DOWN[key];
}
void alt_set_arrow_key_down(int key)
{
    _ALT_ARROW_KEY_DOWN[key] = true;
}
void alt_set_arrow_key_up(int key)
{
    _ALT_ARROW_KEY_DOWN[key] = false;
}

bool left_shift_key_down(void)
{
    return SPECIAL_KEY_DOWN[SpecialKey_LeftShift];
}
bool space_key_down(void)
{
    return SPECIAL_KEY_DOWN[SpecialKey_Space];
}

void key_callback_arrows_down(GLFWwindow *window, int key,
                int scancode, int action,
                int mods)
{
    if (action == GLFW_PRESS) {
        for (int dir = 0; dir < NumDirs; dir++) {
            if (key_is_dir(dir, key)) {
                set_arrow_key_down(dir);
            }
            if (alt_key_is_dir(dir, key)) {
                alt_set_arrow_key_down(dir);
            }
        }
        for (int i = 0; i < NUM_SPECIAL_KEYS; i++) {
            if (key == GLFW_SPECIAL_KEY_MAP[i]) {
                SPECIAL_KEY_DOWN[i] = true;
                break;
            }
        }
    } else if (action == GLFW_RELEASE) {
        for (int dir = 0; dir < NumDirs; dir++) {
            if (key_is_dir(dir, key)) {
                set_arrow_key_up(dir);
            }
            if (alt_key_is_dir(dir, key)) {
                alt_set_arrow_key_up(dir);
            }
        }
        for (int i = 0; i < NUM_SPECIAL_KEYS; i++) {
            if (key == GLFW_SPECIAL_KEY_MAP[i]) {
                SPECIAL_KEY_DOWN[i] = false;
                break;
            }
        }
    }
}

/* #define NUM_KEYS 2 */
/* static bool */
/* void key_callback_extra_down(GLFWwindow *window, int key, */
/*                 int scancode, int action, */
/*                 int mods) */
/* { */
/*     // More key-down state */
/*     int keys[NUM_KEYS] = { GLFW_KEY_Q, */
/*                      GLFW_KEY_E }; */
/*     if (action == GLFW_PRESS) { */
/*         for (int i = 0; i < NUM_KEYS; i++) { */
/*             if (key == */ 
/*         } */
/*     } else if (action == GLFW_RELEASE) { */
/*         for (int i = 0; i < NUM_KEYS; i++) { */

/*         } */
/*     } */
/* #undef NUM_KEYS */
/* } */

void key_callback_quit(GLFWwindow *window, int key,
                int scancode, int action,
                int mods)
{
    if (action == GLFW_PRESS)  {
        if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
    }
}

int number_key(int number)
{
    switch (number) {
        case 0: return GLFW_KEY_0; break;
        case 1: return GLFW_KEY_1; break;
        case 2: return GLFW_KEY_2; break;
        case 3: return GLFW_KEY_3; break;
        case 4: return GLFW_KEY_4; break;
        case 5: return GLFW_KEY_5; break;
        case 6: return GLFW_KEY_6; break;
        case 7: return GLFW_KEY_7; break;
        case 8: return GLFW_KEY_8; break;
        case 9: return GLFW_KEY_9; break;
    }
    return GLFW_KEY_0; // ...
}
