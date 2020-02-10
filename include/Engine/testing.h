/*--------------------------------------------------------------------------------
    Testing stuff
--------------------------------------------------------------------------------*/
#ifndef HEADER_DEFINED_TESTING
#define HEADER_DEFINED_TESTING
void test_spawn_cubes(int n);
void test_floor(char *texture_path);
DirectionalLight *test_directional_light_controlled(void);
DirectionalLight *test_directional_light_auto(void);
void test_point_light_1(void);
void test_mass_objects(int number_of_them);
void test_spawn_star(float x, float y, float z, float theta_x, float theta_y, float theta_z);
void test_spawn_stars(int how_many);

#endif // HEADER_DEFINED_TESTING
