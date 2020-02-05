/*--------------------------------------------------------------------------------
"Gameobject" aspects.

Transform
    3D position, orientation, stored with Euler angles.
Body
    Viewable mesh aspect.
Logic
    Per-frame update logic.
Input
Camera
DirectionalLight
PointLight
--------------------------------------------------------------------------------*/
#include "Engine.h"

void init_aspects_gameobjects(void)
{
    new_default_manager(Transform, NULL);
    new_default_manager(Body, NULL);
    new_default_manager(Logic, NULL);
    new_default_manager(Input, NULL);
    new_default_manager(Camera, NULL);
    new_default_manager(DirectionalLight, NULL);
    new_default_manager(PointLight, NULL);
    new_default_manager(Text, NULL);
}


