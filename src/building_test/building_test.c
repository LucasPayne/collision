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

int main(int argc, char *argv[])
{
    Matrix4x4f mat;
    identity_matrix4x4f(&mat);
    print_matrix4x4f(&mat);

    /* PLY *ply = read_ply(DATA_DIR "models/plytest.ply"); */
    PLY *ply = read_ply(DATA_DIR "models/plytest2.ply");
    /* PLY *ply = read_ply(DATA_DIR "models/dolphins.ply"); */
    print_ply(ply);

    void *data = ply_binary_data(ply);
    for (int i = 0; i < 10; i++) printf("%f\n", *(((float *) data) + i));

    destroy_ply(ply);
}
