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

int main(void)
{
    DD *dd = dd_fopen("data.dd");
    dd_check(dd);
    dd_print(dd);

    DD *appconf = dd_open(dd, "ApplicationConfiguration");
    dd_check(appconf);
    dd_print(appconf);

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
    dd_print(spider);

    DD *scene = dd_open(dd, "Scene");
    dd_print(scene);

    printf("SCANNING SPIDERS\n");
    DD **spiders;
    int num = dd_scan(scene, &spiders, "Spider");
    for (int i = 0; i < num; i++) {
        dd_print(spiders[i]);
    }

    printf("SCANNING DUDES\n");
    DD **dudes;
    num = dd_scan(scene, &dudes, "Dude");
    for (int i = 0; i < num; i++) {
        dd_print(dudes[i]);
    }

    printf("SCANNING GAMEOBJECTS\n");
    DD **gameobjects;
    num = dd_scan(scene, &gameobjects, "GameObject");
    for (int i = 0; i < num; i++) {
        dd_print(gameobjects[i]);
    }

}

