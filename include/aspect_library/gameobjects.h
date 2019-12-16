/*================================================================================
"Gameobject" aspects.
Transform : 3D position, orientation, stored with Euler angles.
Body: Viewable mesh aspect.
Camera: 

Currently, this is not really a "library". A useful "game object" system above
the entity and resource systems should probably only be made by editing this a lot
and then deciding what aspects are useful, then making it a proper library.
================================================================================*/
#ifndef HEADER_DEFINED_ASPECT_LIBRARY_GAMEOBJECTS
#define HEADER_DEFINED_ASPECT_LIBRARY_GAMEOBJECTS
#include "entity.h"
#include "matrix_mathematics.h"
#include "resources.h"

void init_aspects_gameobjects(void);

/*--------------------------------------------------------------------------------
A Transform is the positional aspect of a gameobject. This is stored as
Position: x,y,z,  Eulerangles: theta_x,theta_y,theta_z.

This is shared positional data. All aspects which rely on their entity to
have a place and orientation in the world should share this and get it as a sibling aspect.

There is a routine for getting the model matrix from this transform.
Also, initializing and setting a transform is easy:
    Transform_set(entity_add_aspect(entity, Transform), 0,1,2, 3,4,5);
--------------------------------------------------------------------------------*/
extern AspectType Transform_TYPE_ID;
typedef struct /* Aspect */ Transform_s {
ASPECT_PROPERTIES()
    float x;
    float y;
    float z;
    float theta_x;
    float theta_y;
    float theta_z;
} Transform;
void Transform_set(Transform *transform, float x, float y, float z, float theta_x, float theta_y, float theta_z);
Matrix4x4f Transform_matrix(Transform *transform);

/*--------------------------------------------------------------------------------
A Body is the seeable aspect of a gameobject. This gives information enough
to render the gameobject.

Currently, the only "body" is a mesh under a single graphics program.
----Should put standard body-render loops here.
--------------------------------------------------------------------------------*/
extern AspectType Body_TYPE_ID;
typedef struct /* Aspect */ Body_s {
ASPECT_PROPERTIES()
    ResourceHandle mesh; /* Resource: Mesh */
} Body;
void Body_init(Body *body, char *material_path, char *mesh_path);

/*--------------------------------------------------------------------------------
Logic is the behavioral aspect of a gameobject. It holds an update routine
and optional data. Macros are provided for simple use of this per-entity specific
usage of the logic aspect.

---Should put standard logic loops here.
--------------------------------------------------------------------------------*/
extern AspectType Logic_TYPE_ID;
typedef struct Logic_s {
ASPECT_PROPERTIES()
    void (*update)(struct Logic_s *);
    void *data;
} Logic;
#define init_logic(LOGIC,OBJ_TYPE_NAME,UPDATE)\
{\
    ( LOGIC )->update = ( UPDATE );\
    ( LOGIC )->data = (struct OBJ_TYPE_NAME ## Data *) calloc(1, sizeof(struct OBJ_TYPE_NAME ## Data));\
    mem_check(( LOGIC )->data);\
}
#define object_data(LOGIC,OBJ_TYPE_NAME)\
    ((struct OBJ_TYPE_NAME ## Data *) ( LOGIC )->data)

/*--------------------------------------------------------------------------------
--------------------------------------------------------------------------------*/
extern AspectType Camera_TYPE_ID;
typedef struct /* Aspect */ Camera_s {
ASPECT_PROPERTIES()
    Matrix4x4f projection_matrix; // the lens
} Camera;

#endif // HEADER_DEFINED_ASPECT_LIBRARY_GAMEOBJECTS