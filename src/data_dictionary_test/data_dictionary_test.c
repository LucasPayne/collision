/*
PROJECT_LIBS:
    + data_dictionary
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helper_definitions.h"
#include "data_dictionary.h"

#define dd_check(DDICT)\
{\
    if (( DDICT ) == NULL) {\
        fprintf(stderr, ERROR_ALERT "Could not open data dictionary.\n");\
        exit(EXIT_FAILURE);\
    }\
}
typedef DataDictionary DD;

int main(void)
{
    DD *dd = dd_fopen("data.dd");
    dd_check(dd);
    dd_print_table(dd);
}

