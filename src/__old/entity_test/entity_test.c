/*
 * Quick command line testing schematic
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "entity.h"

#define print_line()\
    printf("================================================================================\n");

void bungus(Entity2D *self)
{
    printf("%s says \"bungus\".\n", self->name);
}

int main(int argc, char *argv[])
{

    init_entity_model();

    EntityID bean = create_entity(ENTITY_UNIVERSE, "bean", bungus, 0, 0, 0);
    create_entity(bean, "quambus", NULL, 0, 0, 0);
    create_entity(bean, "quambus_again", NULL, 0, 0, 0);
    EntityID bean2 = create_entity(ENTITY_UNIVERSE, "bean2", NULL, 0, 0, 0);

    entity_add_component(bean2, 3, "bomp");
    entity_add_component(bean2, 63, "wwwww");

    print_line();
    print_entity_tree(NULL);
    update_entity_model();

    EntityID renderer = create_entity(bean2, "renderer", NULL, 0, 0, 0);
    entity_add_component(renderer, 2, "bee");
    EntityID solid = create_entity(bean2, "solid", bungus, 0, 0, 0);

    create_entity(ENTITY_UNIVERSE, "bean3", NULL, 0, 0, 0);
    create_entity(ENTITY_UNIVERSE, "bean4", NULL, 0, 0, 0);

    print_line();
    print_entity_tree(NULL);
    update_entity_model();
        
    close_entity_model();

    exit(EXIT_SUCCESS);
}
