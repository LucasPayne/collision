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
#include "painting.h"
#include "scenes.h"

#include "shader_blocks/Standard3D.h"
#include "shader_blocks/StandardLoopWindow.h"
#include "shader_blocks/Lights.h"

extern float time;
extern float dt;

extern float ASPECT_RATIO;
extern DataDictionary *g_data;
extern DataDictionary *g_scenes;

//---
/*================================================================================
"Gameobject" aspects.
Transform : 3D position, orientation, stored with Euler angles.
Body: Viewable mesh aspect.
Camera:
Logic:
Input:
DirectionalLight:
PointLight:
================================================================================*/
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
vec3 Transform_position(Transform *t);
Matrix4x4f Transform_matrix(Transform *transform);
vec3 Transform_relative_direction(Transform *t, vec3 direction);
vec3 Transform_relative_position(Transform *t, vec3 position);
void Transform_move(Transform *t, vec3 translation);
void Transform_move_relative(Transform *t, vec3 translation);
vec3 Transform_up(Transform *t);
vec3 Transform_down(Transform *t);
vec3 Transform_left(Transform *t);
vec3 Transform_right(Transform *t);
vec3 Transform_forward(Transform *t);
vec3 Transform_backward(Transform *t);
vec3 Transform_angles(Transform *t);

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
struct Logic_s;
typedef void (*LogicUpdate)(struct Logic_s *);
typedef struct Logic_s {
ASPECT_PROPERTIES()
    LogicUpdate update;
    void *data;
} Logic;
void Logic_init(Logic *logic, LogicUpdate update);

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
    An Input aspect allows the entity to listen for input events.
--------------------------------------------------------------------------------*/
struct Input_s;
typedef void (*KeyListener)(struct Input_s *, int, int, int); // No abstraction, just straight GLFW action, key, and mods.
typedef void (*MousePositionListener)(struct Input_s *, double, double); // x, y position of mouse in GLFW screen units.
typedef void (*MouseMoveListener)(struct Input_s *, double, double); // x, y position of mouse in GLFW screen units.
extern AspectType Input_TYPE_ID;
#define INPUT_KEY 0                // Key press and release.
#define INPUT_MOUSE_POSITION 1     // Mouse position change.
#define INPUT_MOUSE_MOVE 2         // Mouse position change, but given position relative to last mouse position event.
typedef struct /* Aspect */ Input_s {
ASPECT_PROPERTIES()
    bool listening;
    uint8_t input_type;
    union {
        KeyListener key;
        MousePositionListener mouse_position;
        MouseMoveListener mouse_move;
    } callback;
} Input;
void Input_init(Input *inp, uint8_t input_type, /* generic function, type unsafe */ void *callback, bool listening);

/*--------------------------------------------------------------------------------
    A camera aspect causes things to be rendered to the camera's rectangle,
    using a projection matrix derived from its initialization (--and rectangle)
    and view matrix derived via the inverse of its Transform matrix. 
---Depends on Transform.
---todo
    Camera target rectangle, masking rectangle.
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
mat4x4 Camera_vp_matrix(Camera *camera);

/*--------------------------------------------------------------------------------
    Lights
--------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------
    A directional light depends on a transform, but only for its rotation.
--------------------------------------------------------------------------------*/
extern AspectType DirectionalLight_TYPE_ID;
typedef struct DirectionalLight_s {
ASPECT_PROPERTIES()    
    vec4 color;
    float shadow_width;
    float shadow_height;
    float shadow_depth;
} DirectionalLight;
void DirectionalLight_init(DirectionalLight *directional_light, float cr, float cg, float cb, float ca, float shadow_width, float shadow_height, float shadow_depth);
vec3 DirectionalLight_direction(DirectionalLight *directional_light);

/*--------------------------------------------------------------------------------
    Point light, omnidirectional light source.
--------------------------------------------------------------------------------*/
extern AspectType PointLight_TYPE_ID;
typedef struct PointLight_s {
ASPECT_PROPERTIES()
    vec4 color;
    float linear_attenuation;
    float quadratic_attenuation;
    float cubic_attenuation;
} PointLight;
void PointLight_init(PointLight *point_light, float linear_attenuation, float quadratic_attenuation, float cubic_attenuation, float cr, float cg, float cb, float ca);

//================================================================================
// Helper stuff. May separate this.
EntityID create_camera_man(float x, float y, float z, float lookat_x, float lookat_y, float lookat_z);
EntityID create_key_camera_man(float x, float y, float z, float lookat_x, float lookat_y, float lookat_z);

//================================================================================
// Testing stuff.
void test_spawn_cubes(int n);
void test_floor(char *texture_path);
void test_directional_light_auto(void);
void test_directional_light_controlled(void);
void test_point_light_1(void);
void test_mass_objects(int number_of_them);
void test_spawn_star(float x, float y, float z, float theta_x, float theta_y, float theta_z);
void test_spawn_stars(int how_many);

// Applications are not supposed to use this, but it is exposed here for testing.
void ___render(void);


#endif // HEADER_DEFINED_INTERACTIVE_3D
