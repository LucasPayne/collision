/*--------------------------------------------------------------------------------
Application base.
Add this as a library in an application, and it will provide a main function.
The application must provide:
    conf.dd: A file containing application configuration, to be included as app_config. This is written to the ApplicationConfiguration type declared
             in this base's .dd file.

base_libs:
    + glad
    + helper_gl
    + helper_input
    + data_dictionary
    + matrix_mathematics
    + entity
    - aspect_library/gameobjects
    + iterator
    - resources
    - rendering
    - ply
--------------------------------------------------------------------------------*/
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
#include "data_dictionary.h"
/* #include "ply.h" */
/* #include "rendering.h" */
/* #include "resources.h" */
#include "entity.h"
#include "matrix_mathematics.h"
#include "aspect_library/gameobjects.h"

#define BASE_DIRECTORY "/home/lucas/code/collision/lib/bases/interactive_3D/"

extern void init_program(void);

int main(void)
{
    DD *base_config = dd_fopen(BASE_DIRECTORY "interactive_3D.dd");
    DD *app_config = dd_open(base_config, "app_config");

    dd_print(app_config);

    init_program();
}


