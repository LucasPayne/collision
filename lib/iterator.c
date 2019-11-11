/*
 *
 */

#include <stdbool.h>
#include "iterator.h"

void step(Iterator *iterator)
{
    if (iterator->finished) {
        iterator->val = NULL;
        return;
    }
    iterator->coroutine(iterator);
}
void init_iterator(Iterator *iterator, void (*coroutine)(Iterator *))
{
    iterator->coroutine_flag = COROUTINE_START;
    iterator->coroutine = coroutine;
}
void *current(Iterator *iterator)
{
    return iterator->val;
}
