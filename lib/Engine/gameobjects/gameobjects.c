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
    new_default_manager(RigidBody, NULL);
}

// Helper function for creating a typical base gameobject with a transform.
EntityID new_gameobject(float x, float y, float z, float theta_x, float theta_y, float theta_z, bool euler_controlled)
{
    EntityID e = new_entity(4);
    Transform *t = add_aspect(e, Transform);
    Transform_set(t, x, y, z, theta_x, theta_y, theta_z);
    t->euler_controlled = euler_controlled;
    return e;
}
