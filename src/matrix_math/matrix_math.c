/*--------------------------------------------------------------------------------
project_libs:
    + Engine
--------------------------------------------------------------------------------*/
#include "Engine.h"

extern void input_event(int key, int action, int mods)
{
}
extern void mouse_button_event(int button, int action, int mods)
{
}
extern void cursor_move_event(double x, double y)
{
}
extern void init_program(void)
{
    while (1) {
        mat3x3 m;
        // fill_mat3x3_rmaj(m, 8,1,2,3,4,5,6,7,8);
        // fill_mat3x3_rmaj(m, -1,3,-2.5, 8,9,-3.112, 3,3,11);
        // fill_mat3x3_rmaj(m, 2,1,2,-2,3,-1,8,6,7);
        for (int i = 0; i < 9; i++) m.vals[i] = frand()*10-5;
        float det = mat3x3_determinant(m);
        printf("det: %.6f\n", det);
        printf("m:\n");
        print_matrix3x3f(&m);
        mat3x3 minv = mat3x3_inverse(m);
        printf("minv:\n");
        print_matrix3x3f(&minv);
        mat3x3 identity = mat3x3_multiply(m, minv);
        printf("identity:\n");
        print_matrix3x3f(&identity);
        getchar();
    }
}
extern void loop_program(void)
{
    
}
extern void close_program(void)
{
}
