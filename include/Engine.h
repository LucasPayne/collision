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

#include "shader_blocks/Standard3D.h"
#include "shader_blocks/StandardLoopWindow.h"
#include "shader_blocks/Lights.h"

extern float time;
extern float dt;

extern float ASPECT_RATIO;
extern DataDictionary *g_data;
extern DataDictionary *g_scenes;

extern float mouse_x;
extern float mouse_y;

vec2 pixel_to_rect(int pixel_x, int pixel_y, float blx, float bly, float trx, float try); //---

#include "Engine/gameobjects.h"
#include "Engine/game_renderer.h"
#include "Engine/helper.h"
#include "Engine/testing.h"
#include "Engine/scenes.h"

#endif // HEADER_DEFINED_ENGINE
