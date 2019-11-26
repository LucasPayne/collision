/*================================================================================
   An iterator implementation.

   Details:
   There is still a too much work in defining a certain iterator and using an
   iterator. The iterator has a state structure which is passed to it,
   where it uses a flag to jump to a specific label in the coroutine.

   As of writing, its usage is:
       Iterator iterator;
       init_some_defined_iterator(&iterator);
       do {
           step(&iterator);
           ...
       } while (iterator.val != NULL);
================================================================================*/
#ifndef HEADER_DEFINED_ITERATOR
#define HEADER_DEFINED_ITERATOR
#include <stdbool.h>
#include <stdlib.h>

//================================================================================
// Iterator macro definitions. These allow a certain format for iterator
// definitions.
//================================================================================
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

// more convenient, select how many flags you need so the labels don't need to be included in the function
#define BEGIN_COROUTINE_SA(ITERATOR)\
    if (( ITERATOR )->coroutine_flag == COROUTINE_START) {\
        goto coroutine_start;\
    }\
    else if (( ITERATOR )->coroutine_flag == COROUTINE_A) {\
        goto coroutine_a;\
    }\

//================================================================================
// Iterator type definitions
//================================================================================
union iterator_data {
    int int_val;
    void *ptr_val;
    char char_val;
};
typedef struct Iterator_s {
    int coroutine_flag;
    void (*coroutine) (struct Iterator_s *);

    void *val; // currently just a void pointer iterator

    union iterator_data data1;
    union iterator_data data2;
} Iterator;

//================================================================================
// Iterator interface
//================================================================================
void step(Iterator *iterator);
void init_iterator(Iterator *iterator, void (*coroutine)(Iterator *));

#endif // HEADER_DEFINED_ITERATOR
