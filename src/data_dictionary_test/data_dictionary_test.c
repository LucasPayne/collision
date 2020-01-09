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

    DD *appconf = dd_open(dd, "ApplicationConfiguration");
    dd_check(appconf);
    dd_print_table(appconf);

    bool core_profile;
    if (dd_get(appconf, "core_profile", "bool", &core_profile)) {
        if (core_profile) printf("Is core profile.\n");
        else printf("Is not core profile.\n");
    }

    float clear_color[4];
    if (dd_get(appconf, "clear_color", "vec4", &clear_color)) {
        for (int i = 0; i < 4; i++) printf("%.2f\n", clear_color[i]);
    }

    DD *spider = dd_open(dd, "Spider");
    dd_print_table(spider);

    DD *scene = dd_open(dd, "Scene");
    dd_print_table(scene);
    /* DD **spiders; */
    /* int num = dd_scan(scene, &spiders, "Spider"); */
    /* for (int i = 0; i < num; i++) { */
    /*     dd_print_table(spiders[i]); */
    /* } */

}

