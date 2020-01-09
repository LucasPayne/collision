/*
 * Quick command line testing schematic
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "helper_definitions.h"
#include "entity.h"

void bungel_mungus_init(System *self)
{
    printf("BUNGEL MUNGUS IS ACTIVATED\n");
}
void bungel_mungus_update(System *self)
{
    printf("bnugul goin\n");
}
void bungel_mungus_close(System *self)
{

}

int main(int argc, char *argv[])
{
    init_entity_model();
    add_system("bungel mungus", bungel_mungus_init, bungel_mungus_update, bungel_mungus_close);

    EntityID bungus = create_entity(UNIVERSE_ID, "bungus");
        create_entity(bungus, "neat");
        create_entity(bungus, "neato");
    EntityID bungus2 = create_entity(UNIVERSE_ID, "bungus2");
        create_entity(bungus2, "neat");
        EntityID bungus2_renderer = create_entity(bungus2, "renderer");
            entity_add_component(bungus2_renderer, "beapis", 33);
            entity_add_component(bungus2_renderer, "solid", 33);
    create_entity(UNIVERSE_ID, "bungus3");
    EntityID bungus4 = create_entity(UNIVERSE_ID, "bungus4");
        EntityID solid_hull = create_entity(bungus4, "solid_hull");
            entity_add_component(solid_hull, "BUNGUS", 22);
    print_entity_tree();

    update_entity_model();
    update_entity_model();
    update_entity_model();

    close_entity_model();

    exit(EXIT_SUCCESS);
}
