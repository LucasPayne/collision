/*
PROJECT_LIBS:
    + dictionary
    + matrix_mathematics
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "helper_definitions.h"
#include "dictionary.h"
#include "matrix_mathematics.h"

            
bool query_val_mat4x4(char *string, void *data)
{
    Matrix4x4f matrix;
    /* Matrix4x4f *matrix = (Matrix4x4f *) data; */

    puts(string);

    char *p = string;
#define eat() { while (isspace(*p)) { p++; } }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            eat();
            /* printf("*p: %c\n", *p); */
            /* printf("p: %s\n", p); */
            int num_chars;
            if (sscanf(p, "%f%n", &matrix.vals[4*i + j], &num_chars) == EOF) {
                /* printf("bad float\n"); */
                return false;
            }
            p += num_chars;
            eat();
            if (j < 3 && *p++ != ',') {
                /* printf("bad comma\n"); */
                return false;
            } else if (i < 3 && j == 3 && *p++ != ';') {
                /* printf("bad semicolon\n"); */
                return false;
            }
        }
    }
    memcpy(data, &matrix, sizeof(Matrix4x4f));
    return true;
}

void dict_query_rules_matrix_mathematics(DictQuerier *q)
{
    dict_query_rule_add(q, "mat4x4", query_val_mat4x4);
}

int main(void)
{
    printf("i'm in\n");

    FILE *file = fopen("/home/lucas/code/collision/src/query_test/matrix_stuff", "r");
    if (file == NULL) {
        printf("null file\n");
        exit(EXIT_FAILURE);
    }
    Dictionary *d = dictionary_read(file);
    if (d == NULL) {
        printf("null dictionary\n");
        exit(EXIT_FAILURE);
    }
    DictQuerier *q = dict_new_querier(d);
    if (q == NULL) {
        printf("null querier\n");
        exit(EXIT_FAILURE);
    }
    dict_query_rules_matrix_mathematics(q);
    
    Matrix4x4f matrix;
    if (!dict_query_get(q, "mat4x4", "matrix", &matrix)) {
        printf("couldn't get the matrix\n");
        exit(EXIT_FAILURE);
    }
    print_matrix4x4f(&matrix);
}
