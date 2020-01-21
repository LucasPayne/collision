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
vec3 Transform_global_position(Transform *transform);

/*--------------------------------------------------------------------------------
A Body is the seeable aspect of a gameobject. This gives information enough
to render the gameobject.

Currently, this is just a single mesh+material pair. However, later, models
may consist of multiple facets. However that would be a part of the rendering system.
--------------------------------------------------------------------------------*/
extern AspectType Body_TYPE_ID;
typedef struct /* Aspect */ Body_s {
ASPECT_PROPERTIES()
    float scale;
    ResourceHandle material; /* Resource: Material */
    ResourceHandle geometry; /* Resource: Geometry */
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
/* #define init_logic(LOGIC,OBJ_TYPE_NAME,UPDATE)\ */
/* {\ */
/*     ( LOGIC )->update = ( UPDATE );\ */
/*     ( LOGIC )->data = (struct OBJ_TYPE_NAME ## Data *) calloc(1, sizeof(struct OBJ_TYPE_NAME ## Data));\ */
/*     mem_check(( LOGIC )->data);\ */
/* } */
/* #define object_data(LOGIC,OBJ_TYPE_NAME)\ */
/*     ((struct OBJ_TYPE_NAME ## Data *) ( LOGIC )->data) */

#define get_logic_data(DATA_LVALUE,LOGIC_ASPECT_POINTER,DATA_STRUCT)\
    DATA_STRUCT *DATA_LVALUE = (DATA_STRUCT *) ( LOGIC_ASPECT_POINTER )->data

#define init_logic(LOGIC_ASPECT_POINTER,DATA_STRUCT,UPDATE_FUNCTION)\
{\
    ( LOGIC_ASPECT_POINTER )->update = ( UPDATE_FUNCTION );\
    ( LOGIC_ASPECT_POINTER )->data = calloc(1, sizeof(DATA_STRUCT));\
}

// remember each entry in a macro is evaluated for each appearance.
// may be problems with scope, interaction with surrounding text?
#define init_get_logic_data(DATA_LVALUE,LOGIC_ASPECT_POINTER,DATA_STRUCT,UPDATE_FUNCTION)\
{\
    ( LOGIC_ASPECT_POINTER )->update = ( UPDATE_FUNCTION );\
    ( LOGIC_ASPECT_POINTER )->data = calloc(1, sizeof(DATA_STRUCT));\
}\
    DATA_STRUCT *DATA_LVALUE = (DATA_STRUCT *) ( LOGIC_ASPECT_POINTER )->data

    /* mem_check(( LOGIC_ASPECT_POINTER )->data);\ */

/*--------------------------------------------------------------------------------
    An Input aspect allows the entity to listen for input events (if something in the application
    sends those events through to the Input aspects).
--------------------------------------------------------------------------------*/
struct Input_s;
typedef void (*InputCallback)(struct Input_s *, int, int, int); // No abstraction, just straight GLFW action, key, and mods.
extern AspectType Input_TYPE_ID;
typedef struct /* Aspect */ Input_s {
ASPECT_PROPERTIES()
    bool listening;
    InputCallback callback;
} Input;
void Input_init(Input *inp, InputCallback callback, bool listening);

/*--------------------------------------------------------------------------------
--------------------------------------------------------------------------------*/
extern AspectType Camera_TYPE_ID;
typedef struct /* Aspect */ Camera_s {
ASPECT_PROPERTIES()
    float plane_r;
    float plane_l;
    float plane_t;
    float plane_b;
    float plane_n;
    float plane_f;
    Matrix4x4f projection_matrix; // the lens
} Camera;
void Camera_init(Camera *camera, float aspect_ratio, float near_half_width, float near, float far);

#endif // HEADER_DEFINED_ASPECT_LIBRARY_GAMEOBJECTS
