/*================================================================================
    RigidBody
================================================================================*/
#include "Engine.h"

AspectType RigidBody_TYPE_ID;
void RigidBody_init(RigidBody *rb, char *mesh_path, float mass)
{
    rb->geometry = new_resource_handle(Geometry, mesh_path);
    rb->mass = mass;
    // Calculate the inertia tensor assuming that the rigid body has uniform mass.
    
}

