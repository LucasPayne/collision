#ifndef HEADER_DEFINED_INTERACTIVE_3D
#define HEADER_DEFINED_INTERACTIVE_3D
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
#include "aspect_library/gameobjects.h"

typedef Matrix4x4f mat4x4;
#include "shader_blocks/Standard3D.h"
#include "shader_blocks/StandardLoopWindow.h"
#include "shader_blocks/Lights.h"

extern float time;
extern float dt;

extern float ASPECT_RATIO;
extern DataDictionary *g_data;

#endif // HEADER_DEFINED_INTERACTIVE_3D
