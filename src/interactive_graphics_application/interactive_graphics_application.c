/*--------------------------------------------------------------------------------
project_libs:
    + bases/interactive_3D
--------------------------------------------------------------------------------*/
#include <stdio.h>


extern void init_program(void)
{
    printf("i'm in\n");
}

extern void loop_program(void)
{
    printf("i'm goin\n");
}

extern void close_program(void)
{
    printf("i'm out\n");
}
