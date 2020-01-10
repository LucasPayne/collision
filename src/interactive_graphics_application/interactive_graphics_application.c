/*--------------------------------------------------------------------------------
project_libs:
    + bases/interactive_3D
--------------------------------------------------------------------------------*/
#include <stdio.h>
#include "bases/interactive_3D.h"

const float ASPECT_RATIO = 0.5616;

extern void init_program(void)
{
    EntityID camera_man = new_entity(4);
    Transform_set(entity_add_aspect(camera_man, Transform), 0,0,0,0,0,0);
    Camera *camera = entity_add_aspect(camera_man, Camera);
    Camera_init(camera, ASPECT_RATIO, 1, 0.9, 10);
}

extern void loop_program(void)
{
    for_aspect(Camera, camera)
        Transform *t = get_sibling_aspect(camera, Transform);
        printf("%.2f, %.2f, %.2f\n", t->x, t->y, t->z);
    end_for_aspect()
}

extern void close_program(void)
{
    printf("i'm out\n");
}
