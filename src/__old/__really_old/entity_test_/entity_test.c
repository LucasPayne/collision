/*================================================================================
PROJECT_LIBS:
    + entity
    + iterator
 ================================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "helper_definitions.h"
#include "iterator.h"
#include "entity.h"

typedef struct AspectA_s {
ASPECT_PROPERTIES()
    int a;
    int b;
    int c;
    int d;
} AspectA;
void AspectA_serialize (FILE *file, void *self)
{
    fprintf(file, "a: %d\n", ((AspectA *) self)->a);
    fprintf(file, "b: %d\n", ((AspectA *) self)->b);
    fprintf(file, "c: %d\n", ((AspectA *) self)->c);
    fprintf(file, "d: %d\n", ((AspectA *) self)->d);
}

typedef struct AspectB_s {
ASPECT_PROPERTIES()
    char stuff[12];
    int thing;
} AspectB;
typedef struct AspectC_s {
ASPECT_PROPERTIES()
    float floats[10];
    float floats2[3];
} AspectC;
typedef struct AspectD_s {
ASPECT_PROPERTIES()
    float x;
    float y;
    float z;
} AspectD;
void AspectD_serialize (FILE *file, void *self)
{
    fprintf(file, "x: %f\n", ((AspectD *) self)->x);
    fprintf(file, "y: %f\n", ((AspectD *) self)->y);
    fprintf(file, "z: %f\n", ((AspectD *) self)->z);
}

#define AspectA_TYPE_ID 1
#define AspectB_TYPE_ID 2
#define AspectC_TYPE_ID 3
#define AspectD_TYPE_ID 4

int main(int argc, char *argv[])
{
    init_entity_model();
    //...
    /* aspect_type_sizes[0] = sizeof(AspectA); */
    /* aspect_type_sizes[1] = sizeof(AspectB); */
    /* aspect_type_sizes[2] = sizeof(AspectC); */
    /* aspect_type_sizes[3] = sizeof(AspectD); */

    /* printf("It apparently initialized!\n"); */
    /* printf("Entities after initialization:\n"); */
    /* print_entities(); */

    EntityID entity = new_entity(10);
    /* printf("Entities after making one:\n"); */
    /* print_entities(); */

    for (int i = 0; i < 6; i++) {
        new_entity(i * i + 2);
    }
    /* printf("Entities after making more:\n"); */
    /* print_entities(); */

    new_manager(AspectA, default_manager_new_aspect, default_manager_destroy_aspect, default_manager_aspect_iterator, AspectA_serialize);
    /* printf("adding aspect ...\n"); */
    AspectA *a = entity_add_aspect(entity, AspectA);
    a->a = 1;
    a->b = -1;
    a->c = 4444;
    a->d = 33;
    /* printf("added aspect\n"); */
    /* print_aspects_of_type(AspectA); */

    new_manager(AspectB, default_manager_new_aspect, default_manager_destroy_aspect, default_manager_aspect_iterator, NULL);
    EntityID entity2 = new_entity(2);
    for (int i = 0; i < 4; i++) {
        AspectA *a = entity_add_aspect(entity, AspectA);
        a->a = 1;
        a->b = -1;
        a->c = 4444;
        a->d = 33;
        entity_add_aspect(entity2, AspectB);
    }
    /* printf("added more\n"); */
    /* print_aspects_of_type(AspectA); */
    /* print_aspects_of_type(AspectB); */

    new_manager(AspectD, default_manager_new_aspect, default_manager_destroy_aspect, default_manager_aspect_iterator, AspectD_serialize);
    for (int i = 0; i < 4000; i++) {
        EntityID e = new_entity(3);
        AspectD *d = entity_add_aspect(e, AspectD);
        d->x = frand();
        d->y = frand();
        d->z = frand();
        AspectA *a = entity_add_aspect(e, AspectA);
        a->a = 18;
        a->b = 12;
        a->c = 11;
        a->d = 0;
        /* entity_add_aspect(entity2, AspectB); */
    }
    /* printf("added way more\n"); */
    /* print_aspects_of_type(AspectD); */
    print_entities();

    exit(EXIT_SUCCESS);
}
