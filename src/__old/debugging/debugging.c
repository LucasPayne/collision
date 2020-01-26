/*--------------------------------------------------------------------------------
project_libs:
    + bases/gl_debug
--------------------------------------------------------------------------------*/
#include <stdio.h>
#include "bases/gl_debug.h"

const float ASPECT_RATIO = 0.5616;

extern void init_program(void)
{
    printf("i'm in\n");

    GLuint p = glCreateProgram();
    GLuint v = glCreateShader(GL_VERTEX_SHADER);
    GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
    load_and_compile_shader(v, "passthrough_none.vert");
    load_and_compile_shader(f, "red.frag");
    // Attach the shaders. If they aren't loaded resources, then this loads and tries to compile them.
    glAttachShader(p, v);
    glAttachShader(p, f);
    // Link the program, and do error-checking.
    link_shader_program(p);
    // Detach the shaders so they can be deleted if needed.
    glDetachShader(p, v);
    glDetachShader(p, f);
}

extern void loop_program(void)
{
    printf("going\n");
}

extern void close_program(void)
{
    printf("i'm out\n");
}
