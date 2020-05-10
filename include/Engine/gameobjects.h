/*================================================================================
"Gameobject" aspects.
Transform : 3D position, orientation. Optionally controlled by Euler angles.
Body: Viewable mesh aspect.
Camera:
Logic:
DirectionalLight:
PointLight:
// Input --- Removed, this is now merged into the Logic aspect.
================================================================================*/
#ifndef HEADER_DEFINED_GAMEOBJECTS
#define HEADER_DEFINED_GAMEOBJECTS
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
    mat3x3 rotation_matrix;
    float x;
    float y;
    float z;
    //--- This is here for backward compatability. Maybe rethink how a transform can be controlled by angles with no build-up of numerical error.
    //--- Possibly will be easier when Logic aspects can be modular and have modular data (as in "euler controller" as a sort of mini-aspect).
    bool euler_controlled;
    float theta_x;
    float theta_y;
    float theta_z;
    vec3 center;
    float scale;

    bool has_parent;
    struct Transform_s *parent; // Optional parent. Matrices and world positions are concatenated.
} Transform;
void Transform_set(Transform *transform, float x, float y, float z, float theta_x, float theta_y, float theta_z);
void Transform_set_position(Transform *transform, vec3 position);
vec3 Transform_position(Transform *t);
mat4x4 Transform_matrix(Transform *transform);
mat3x3 Transform_rotation_matrix(Transform *t);
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
void Transform_draw_axes(Transform *t, float length, float width);

// Utility macro for setting the painting matrix with this transform.
#define Transform_painting_matrix(TRANSFORM) painting_matrix(Transform_matrix(TRANSFORM))

// Utility stuff, since Transforms are used a lot. Maybe it would actually be better to have a transform as an intrinsic
// part of an entity, even if it might not be used.
#define Transform_get(ENTITY_ID) get_aspect_type(ENTITY_ID, Transform)
#define Transform_get_a(ASPECT) get_sibling_aspect(ASPECT, Transform)
#define Transform_get_matrix(ENTITY_ID) Transform_matrix(get_aspect_type(ENTITY_ID, Transform))
#define Transform_get_matrix_a(ASPECT) Transform_matrix(get_sibling_aspect(ASPECT, Transform))
#define Transform_get_position(ENTITY_ID) Transform_position(get_aspect_type(ENTITY_ID, Transform))
#define Transform_get_position_a(ASPECT) Transform_position(get_sibling_aspect(ASPECT, Transform))

/*--------------------------------------------------------------------------------
A Body is the seeable aspect of a gameobject. This gives information enough
to render the gameobject.

Currently, this is just a single mesh+material pair. However, later, models
may consist of multiple facets. However that would be a part of the rendering system.
--------------------------------------------------------------------------------*/
extern AspectType Body_TYPE_ID;
typedef struct /* Aspect */ Body_s {
ASPECT_PROPERTIES()
    bool visible;
    // float scale;
    ResourceHandle material; /* Resource: Material */
    ResourceHandle geometry; /* Resource: Geometry */
    bool is_ground;
} Body;
void Body_init(Body *body, char *material_path, char *mesh_path);
float Body_radius(Body *body);
mat4x4 Body_matrix(Body *body);

/*--------------------------------------------------------------------------------
A RigidBody is simulated according to rigid body dynamics. The geometry need not be the
same as what the object is being rendered as. Currently rigid bodies are restricted to being
convex polyhedra, although the algorithms can be generalized to convex sets, and constraints can
be incorporated along with convex decomposition to allow concave objects.
--------------------------------------------------------------------------------*/
typedef uint8_t RigidBodyType;
typedef enum RigidBodyTypes {
    RigidBodyPolytope,
    NUM_RIGID_BODY_TYPES
};
extern AspectType RigidBody_TYPE_ID;
typedef struct /* Aspect */ RigidBody_s {
ASPECT_PROPERTIES()
    RigidBodyType type;
    union {
        struct {
            // Given as a point cloud, as any other structure is not needed.
            vec3 *points;
            int num_points;
        } polytope;
    } shape;
    vec3 linear_momentum;

    vec3 angular_momentum;

    float mass;
    float inverse_mass;
    vec3 center_of_mass;
    mat3x3 inertia_tensor;
    mat3x3 inverse_inertia_tensor;
} RigidBody;
void RigidBody_init_polytope(RigidBody *rb, vec3 *points, int num_points, float mass);


/*--------------------------------------------------------------------------------
Logic is the behavioural aspect of a gameobject. It holds an update routine
and optional data. Macros are provided for simple use of this per-entity specific
usage of the logic aspect.

The Logic aspect also handles input listening.
--- This was previously handled by a separate Input aspect.

---Should put standard logic loops here.
--------------------------------------------------------------------------------*/
struct Logic_s;
typedef void (*KeyListener)(struct Logic_s *, int, int, int); // No abstraction, just straight GLFW action, key, and mods.
typedef void (*MousePositionListener)(struct Logic_s *, float, float); // x, y position of mouse.
typedef void (*MouseMoveListener)(struct Logic_s *, float, float, float, float); // x, y position of mouse, dx, dy relative change in position of mouse.
typedef void (*MouseButtonListener)(struct Logic_s *, MouseButton, bool, float, float); // Button, click=true: Pressed ; click=false: Released, mouse x, mouse y
typedef void (*ScrollListener)(struct Logic_s *, float); // Y offset movement from the scroll wheel.

enum InputTypes {
    INPUT_KEY,                // Key press and release.
    INPUT_MOUSE_POSITION,     // Mouse position change.
    INPUT_MOUSE_MOVE,         // Mouse position change, but given position relative to last mouse position event.
    INPUT_MOUSE_BUTTON,       // Mouse button press and release.
    INPUT_SCROLL_WHEEL,       // Scroll wheel movement.
};

extern AspectType Logic_TYPE_ID;
typedef void (*LogicUpdate)(struct Logic_s *);
typedef struct Logic_s {
ASPECT_PROPERTIES()
    bool updating;
    LogicUpdate update;
    void *data;

    bool key_listening;
    bool mouse_position_listening;
    bool mouse_move_listening;
    bool mouse_button_listening;
    bool scroll_listening;
    KeyListener key_listener;
    MousePositionListener mouse_position_listener;
    MouseMoveListener mouse_move_listener;
    MouseButtonListener mouse_button_listener;
    ScrollListener scroll_listener;
} Logic;
void Logic_init(Logic *logic, LogicUpdate update);

Logic *___add_logic(EntityID entity, LogicUpdate update_function, size_t data_size);
#define add_logic(ENTITY_ID,UPDATE_FUNCTION,LOGIC_DATA_STRUCT_NAME)\
    ___add_logic(ENTITY_ID,UPDATE_FUNCTION, sizeof(LOGIC_DATA_STRUCT_NAME))
Logic *add_empty_logic(EntityID entity, LogicUpdate update_function);

void Logic_add_input(Logic *logic, uint8_t input_type, void *callback);


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
    float aspect_ratio;
    float blx;
    float bly;
    float trx;
    float try; // screen subrectangle in screen coordinates, bottom-left (0,0) to top-right (1,1).
    bool override_bg_color;
    vec4 bg_color;
    mat4x4 projection_matrix; // the lens
} Camera;
void Camera_init(Camera *camera, float aspect_ratio, float near_half_width, float near, float far);
mat4x4 Camera_vp_matrix(Camera *camera);
// Prepare the camera for rendering (upload uniform information, etc.) and return the view-projection matrix.
mat4x4 Camera_prepare(Camera *camera);
// Get the origin and direction of a ray from the camera, where (x,y) are camera screen coordinates,
// with (0,0) the bottom-left, and (1,1) the top-right. The ray starts on the near plane.
void Camera_ray(Camera *camera, float x, float y, vec3 *origin, vec3 *direction);

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

/*--------------------------------------------------------------------------------
    Text
--------------------------------------------------------------------------------*/
typedef uint8_t TextType;
enum TextTypes {
    TextPlaced,   // a quad in 3D which the text is placed onto.
    TextOriented, // a quad which is oriented toward the camera and projected orthographically, where the text is rendered is an overlay.
    TextOrientedFixed, // the same as above except the text does not scale due to the depth.
    Text2D,       // the transform aspect is interpreted as 2D coordinates, placing the text as a GUI overlay.
};
extern AspectType Text_TYPE_ID;
typedef struct Text_s {
ASPECT_PROPERTIES()
    // interface
    TextType type;
    char *string;
    ResourceHandle font; // Resource: Font
    float scale;
    // implementation
    Geometry geometry;
} Text;
void Text_bake(Text *text);
void Text_init(Text *text, TextType type, char *font_path, char *string, float scale);
void Text_set(Text *text, char *string);
void Text_render(mat4x4 matrix, Text *text);

//--------------------------------------------------------------------------------
// Utility functions.
//--------------------------------------------------------------------------------
// Helper function for creating a typical base gameobject with a transform.
EntityID new_gameobject(float x, float y, float z, float theta_x, float theta_y, float theta_z, bool euler_controlled);

#endif // HEADER_DEFINED_GAMEOBJECTS
