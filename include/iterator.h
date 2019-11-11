/*
 * An iterator implementation
 */

#ifndef HEADER_DEFINED_ITERATOR
#define HEADER_DEFINED_ITERATOR

#include <stdbool.h>
#include <stdlib.h>

#define COROUTINE_NULL -1
#define COROUTINE_START 0
#define COROUTINE_A 1
#define COROUTINE_B 2
#define COROUTINE_C 3
#define COROUTINE_D 4
#define COROUTINE_E 5

#define BEGIN_COROUTINE(ITERATOR)\
    if (( ITERATOR )->coroutine_flag == COROUTINE_START) {\
        goto coroutine_start;\
    }\
    else if (( ITERATOR )->coroutine_flag == COROUTINE_A) {\
        goto coroutine_a;\
    }\
    else if (( ITERATOR )->coroutine_flag == COROUTINE_B) {\
        goto coroutine_b;\
    }\
    else if (( ITERATOR )->coroutine_flag == COROUTINE_C) {\
        goto coroutine_c;\
    }\
    else if (( ITERATOR )->coroutine_flag == COROUTINE_D) {\
        goto coroutine_d;\
    }\
    else if (( ITERATOR )->coroutine_flag == COROUTINE_E) {\
        goto coroutine_e;\
    }

            
typedef struct Iterator_s {
    int coroutine_flag;
    void (*coroutine) (struct Iterator_s *);

    void *val; // currently just a void pointer iterator

    union data1_union {
        int int_val;
        void *ptr_val;
        char char_val;
    } data1;
    union data2_union {
        int int_val;
        void *ptr_val;
        char char_val;
    } data2;
} Iterator;

void step(Iterator *iterator);
void *current(Iterator *iterator);
void init_iterator(Iterator *iterator, void (*coroutine)(Iterator *));

#endif // HEADER_DEFINED_ITERATOR
