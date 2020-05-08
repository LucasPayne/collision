#ifndef HEADER_DEFINED_ENGINE
#define HEADER_DEFINED_ENGINE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "helper_definitions.h"
#include "helper_gl.h"
#include "helper_input.h"
#include "memory.h"
#include "data_dictionary.h"
#include "ply.h"
#include "rendering.h"
#include "resources.h"
#include "entity.h"
#include "matrix_mathematics.h"
#include "painting.h"
#include "scenes.h"
#include "geometry.h"

#include "shader_blocks/Standard3D.h"
#include "shader_blocks/StandardLoopWindow.h"
#include "shader_blocks/Lights.h"

extern float g_bg_color[]; //[4]
extern int g_window_width;
extern int g_window_height;
extern int g_subwindow_blx;
extern int g_subwindow_bly;
extern int g_subwindow_trx;
extern int g_subwindow_try;

extern float time;
extern float dt;

extern float ASPECT_RATIO;
extern DataDictionary *g_data;
extern DataDictionary *g_scenes;

// Top-level input, after processing GLFW events.
enum MouseButtons {
    MouseLeft,
    MouseRight
};
typedef uint8_t MouseButton;
extern float mouse_x;
extern float mouse_y;
extern float mouse_screen_x;
extern float mouse_screen_y;

extern GLenum g_cull_mode;

extern int TEST_SWITCH;

vec2 pixel_to_rect(int pixel_x, int pixel_y, float blx, float bly, float trx, float try); //---

#include "Engine/gameobjects.h"
#include "Engine/game_renderer.h"
#include "Engine/helper.h"
#include "Engine/testing.h"
#include "Engine/scenes.h"
#include "Engine/collision.h"

#endif // HEADER_DEFINED_ENGINE
