#ifndef HEADER_DEFINED_PLAYER
#define HEADER_DEFINED_PLAYER

enum FlyingModes {
    PlayerFly,
    PlayerFlyWalk, // This allows toggling.
    PlayerWalk,
};

typedef struct PlayerController_s {
    // Collider *collider;
    vec3 velocity;

    uint8_t flying_mode;

    bool look_with_mouse; // Otherwise, look with the regular keys.
    
    // The azimuth is measured anti-clockwise.
    // It is the angle (1,0,0) makes with its image under the orientation matrix.
    float azimuth;
    // Altitude is measured upward. It is the angle that (0,0,1) makes with its image under the orientation matrix.
    float altitude;

    bool flying;
    float toggle_flying_timer;
} PlayerController;

void PlayerController_key_listener(Logic *g, int key, int action, int mods);
void PlayerController_update(Logic *g);
// void Player_create(vec3 position, float azimuth, float altitude, float camera_near_plane, float camera_far_plane, float camera_near_half_width);

#endif // HEADER_DEFINED_PLAYER
