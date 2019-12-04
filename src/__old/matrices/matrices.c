/*
 * PROJECT_LIBS:
 *      + matrix_mathematics
 *
 * Matrix testing
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "matrix_mathematics.h"

int main(int argc, char *argv[])
{

    Matrix4x4f A = {
       .vals = { 1, 2, 3, 4,
                 3, 1, 0.3, 0.1,
                 0.5, 0.5, 0.5, 0.5
                 -1, 1, -0.2, 0.2 }
    };
    printf("A:\n");
    print_matrix4x4f(&A);

    Matrix4x4f B = {
       .vals = { 1, 2, 3, 4,
                 1, 1, 0.3, 0.1,
                 0.5, 5, 0.5, 0.6
                 -1, 9, -0.3, 0.2 }
    };
    printf("B:\n");
    print_matrix4x4f(&B);

    right_multiply_matrix4x4f(&A, &B);
    printf("AB:\n");
    print_matrix4x4f(&A);

    printf("(1, 2, 3) Eulerian rotation matrix:\n");
    Matrix4x4f rot;
    euler_rotation_matrix4x4f(&rot, 1, 2, 3);
    print_matrix4x4f(&rot);

    exit(EXIT_SUCCESS);
}

