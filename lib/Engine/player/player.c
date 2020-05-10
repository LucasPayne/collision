// A player controller with first person mouse controls, etc. This is meant to be a standard user interface for
// testing graphics applications.
// So, things should be toggleable. Flying toggle, no flying, speed controls, ability to detach the mouse, etc.
#include "Engine.h"

static void lock_altitude(PlayerController *player)
{
    float altitude_up_cap = M_PI / 2 - 0.3;
    float altitude_down_cap = -altitude_up_cap;
    if (player->altitude > altitude_up_cap) player->altitude = altitude_up_cap;
    if (player->altitude < altitude_down_cap) player->altitude = altitude_down_cap;
}
void PlayerController_mouse_move_listener(Logic *g, float x, float y, float dx, float dy)
{
    PlayerController *player = g->data;
    float mouse_sensitivity = 2;
    if (player->look_with_mouse) {
        player->azimuth += dx * mouse_sensitivity;
        player->altitude -= dy * mouse_sensitivity;
        lock_altitude(player);
    }
}

void PlayerController_key_listener(Logic *g, int key, int action, int mods)
{
    PlayerController *player = g->data;
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_E) {
            player->look_with_mouse = !player->look_with_mouse;
        }
    }
}
void PlayerController_scroll_listener(Logic *g, float dy)
{
    PlayerController *player = g->data;
    if (player->scrollable_speed) {
        if (player->speed > player->max_speed) player->speed = player->max_speed;
        if (player->speed < player->min_speed) player->speed = player->min_speed;
        player->speed += 10*dy;
    }
}

void PlayerController_update(Logic *g)
{
    PlayerController *player = g->data;
    Transform *t = Transform_get_a(g);

    float gravitational_acceleration = 9.81;
    if (!player->flying) Y(player->velocity) -= gravitational_acceleration * dt;

    mat3x3 orientation;
    float caz = cos(player->azimuth);
    float saz = sin(player->azimuth);
    float cal = cos(player->altitude);
    float sal = sin(player->altitude);
    fill_mat3x3_cmaj(orientation, caz, 0, saz,
                                  sal*saz, cal, -sal*caz,
                                  -cal*saz, sal, caz*cal);
    t->rotation_matrix = orientation;

    vec3 right = new_vec3(caz, 0, saz);
    vec3 forward = new_vec3(-saz, 0, caz);
    float move_x = 0, move_z = 0;
    if (alt_arrow_key_down(Right)) move_x += player->speed * dt;
    if (alt_arrow_key_down(Left)) move_x -= player->speed * dt;
    if (alt_arrow_key_down(Up)) move_z -= player->speed * dt;
    if (alt_arrow_key_down(Down)) move_z += player->speed * dt;
    Transform_move(t, vec3_add(vec3_mul(right, move_x), vec3_mul(forward, move_z)));

    float vertical_flying_speed = player->speed * 2.0/3.0;
    if (player->flying) {
        if (space_key_down()) {
            t->y += dt * vertical_flying_speed;
        }
        if (left_shift_key_down()) {
            t->y -= dt * vertical_flying_speed;
        }
    }

    if (!player->look_with_mouse) {
        float look_speed = 4;
        if (arrow_key_down(Right)) player->azimuth += look_speed * dt;
        if (arrow_key_down(Left)) player->azimuth -= look_speed * dt;
        if (arrow_key_down(Down)) {
            player->altitude += look_speed * dt;
            lock_altitude(player);
        }
        if (arrow_key_down(Up)) {
            player->altitude -= look_speed * dt;
            lock_altitude(player);
        }
    }
}

void Player_create_default(float x, float y, float z, float azimuth, float altitude)
{
    EntityID e = new_entity(4);
    Transform *t = add_aspect(e, Transform);
    Transform_set(t, x,y,z, 0,0,0);
    Logic *logic = add_logic(e, PlayerController_update, PlayerController);
    Logic_add_input(logic, INPUT_KEY, PlayerController_key_listener);
    Logic_add_input(logic, INPUT_MOUSE_MOVE, PlayerController_mouse_move_listener);
    Logic_add_input(logic, INPUT_SCROLL_WHEEL, PlayerController_scroll_listener);

    Camera *camera = entity_add_aspect(e, Camera);
    Camera_init(camera, ASPECT_RATIO, 10, 10, 1200);

    PlayerController *player = logic->data;
    player->flying_mode = PlayerFlyWalk;
    player->flying = true;
    player->look_with_mouse = true;

    player->speed = 150;
    player->min_speed = player->speed * 0.1;
    player->max_speed = player->speed * 10;
    player->scrollable_speed = true;

    // The player collider is a capsule. This means that the player can walk up things like stairs.
    // Model player_capsule = make_capsule(0.6, 1.9);
    // Collider *collider = add_collider(player, player_capsule.vertices, player_capsule.num_vertices, true);
}
