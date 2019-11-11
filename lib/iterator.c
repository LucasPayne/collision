/*
 *
 */

#include <stdbool.h>
#include "iterator.h"

void step(Iterator *iterator)
{
    iterator->coroutine(iterator);
}
void init_iterator(Iterator *iterator, void (*coroutine)(Iterator *))
{
    iterator->coroutine_flag = COROUTINE_START;
    iterator->coroutine = coroutine;
    iterator->val = NULL;
}
void *current(Iterator *iterator)
{
    return iterator->val;
}
