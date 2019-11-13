/*--------------------------------------------------------------------------------
    Definitions for the iterator module.
    See the header for details.
---------------------------------------------------------------------------------*/
#include <stdbool.h>
#include <string.h>
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
    memset(&iterator->data1, 0, sizeof(union iterator_data));
    memset(&iterator->data2, 0, sizeof(union iterator_data));
}
