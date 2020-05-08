/*--------------------------------------------------------------------------------
PROJECT_LIBS:
	+ glad
	+ text_processing
	+ matrix_mathematics
        - testing/lextest
        + ply
--------------------------------------------------------------------------------*/
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "project_definitions.h"
#include "text_processing.h"
#include "matrix_mathematics.h"
/* #include "testing/lextest.h" */
#include "ply.h"

static void test_file(char *name) {
    /* print_ply(ply); */
    /* void *data = ply_binary_data(ply); */
    /* for (int i = 0; i < 10; i++) printf("%f\n", *(((float *) data) + i)); */

    /* char *query_string = "[vertex]: float x, float y, float z"; */


    PLY *ply = read_ply(name);

    char *color_query = "[position|vertex|positions|vertices]: float r|red, float g|green, float b|blue";
    float *color_data = (float *) ply_get(ply, color_query);
    for (int i = 0; i < 10; i++) printf("%f\n", color_data[i]);

    char *query_string = "[position|vertex|positions|vertices]: \
float x|xpos|xposition|x_pos|posx|pos_x|x_position, \
float y|ypos|yposition|y_pos|posy|pos_y|y_position, \
float z|zpos|zposition|z_pos|posz|pos_z|z_position";
    float *pos_data = (float *) ply_get(ply, query_string);
    for (int i = 0; i < 10; i++) printf("%f\n", pos_data[i]);



    /* PLYQuery *query = read_ply_query(query_string); */
    /* print_ply_query(query); */

    free(pos_data);
    free(color_data);
    destroy_ply(ply);
    /* destroy_ply_query(query); */
}

int main(int argc, char *argv[])
{
    mat4x4 mat;
    identity_matrix4x4f(&mat);
    print_matrix4x4f(&mat);

    /* PLY *ply = read_ply(DATA_DIR "models/plytest.ply"); */
    /* PLY *ply = read_ply(DATA_DIR "models/plytest2.ply"); */

    test_file(DATA_DIR "models/plytest.ply");
    /* getchar(); */
    /* test_file(DATA_DIR "models/plytest2.ply"); */
    /* getchar(); */
    /* test_file(DATA_DIR "models/dolphins.ply"); */

}
