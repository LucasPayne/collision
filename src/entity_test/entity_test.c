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

    Entity2D *bean = create_empty_entity(NULL, "bean");
    create_empty_entity(bean, "quambus");
    create_empty_entity(bean, "quambus_again");
    Entity2D *bean2 = create_empty_entity(NULL, "bean2");

    bean2->update = bungus;

    print_line();
    print_entity_tree(NULL);
    update_entity_model();

    create_empty_entity(bean2, "renderer");
    Entity2D *solid = create_empty_entity(bean2, "solid");

    solid->update = bungus;

    create_empty_entity(NULL, "bean3");
    create_empty_entity(NULL, "bean4");

    print_line();
    print_entity_tree(NULL);
    update_entity_model();
        
    close_entity_model();

    exit(EXIT_SUCCESS);
}
