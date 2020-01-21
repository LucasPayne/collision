/*--------------------------------------------------------------------------------
--------------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "helper_definitions.h"
#include "data_dictionary.h"
#include "resources.h"
#include "entity.h"
#include "scenes.h"
#include "aspect_library/gameobjects.h"
#include "matrix_mathematics.h"
#include "rendering.h"

typedef bool (*AspectReader)(DataDictionary *, void *);
#define new_reader(NAME) bool read_aspect_ ## NAME(DataDictionary *aspect_dd, void *data)
new_reader(Transform) {
    vec3 position;
    if (!dd_get(aspect_dd, "position", "vec3", &position)) return false;
    vec3 rotation; // euler angles
    if (!dd_get(aspect_dd, "rotation", "vec3", &rotation)) return false;
    Transform *transform = (Transform *) data;
    transform->x = position.vals[0];
    transform->y = position.vals[1];
    transform->z = position.vals[2];
    transform->theta_x = rotation.vals[0];
    transform->theta_y = rotation.vals[1];
    transform->theta_z = rotation.vals[2];
    return true;
}
new_reader(Body) {
    float scale;
    if (!dd_get(aspect_dd, "scale", "float", &scale)) return false;
    Body *body = (Body *) data;

//----dummy for now
    Material *mat = oneoff_resource(Material, body->material);
    mat->material_type = new_resource_handle(MaterialType, "Materials/red");
    body->geometry = new_resource_handle(Geometry, "Models/quad");

    body->scale = scale;
    return true;
}
#define reader(NAME) { #NAME, &NAME ## _TYPE_ID, read_aspect_ ## NAME }
static struct { char *aspect_name; AspectType *aspect_type_id_ptr; AspectReader read_aspect; } aspect_readers[] = {
    reader(Transform),
    reader(Body),
    { NULL, NULL, NULL }
};


void open_scene(DD *dict, char *path)
{
    DD *scene_dictionary = dd_open(dict, path);
    if (scene_dictionary == NULL) {
        fprintf(stderr, ERROR_ALERT "Could not open scene.\n");
	exit(EXIT_FAILURE);
    }

    DD **gameobject_dictionaries;
    int num_gameobjects = dd_scan(scene_dictionary, &gameobject_dictionaries, "GameObject");
    printf("num_gameobjects: %d\n", num_gameobjects);
    for (int i = 0; i < num_gameobjects; i++) {
        printf("GameObject %d:\n", i);

        // Defined aspect readers determine what dictionary types are looked for, and how they are read into aspect data to be attached to the entity.
        EntityID entity = new_entity(3);
        for (int j = 0; aspect_readers[j].aspect_name != NULL; j++) {
            DD **aspect_dictionaries;
            int num_aspects = dd_scan(gameobject_dictionaries[i], &aspect_dictionaries, aspect_readers[j].aspect_name);
            for (int k = 0; k < num_aspects; k++) {
                AspectType aspect_type = *aspect_readers[j].aspect_type_id_ptr;
                AspectID aspect = _entity_add_aspect(entity, aspect_type);
                void *data = get_aspect_data(aspect);
	        if (!aspect_readers[j].read_aspect(aspect_dictionaries[k], data)) {
                    fprintf(stderr, ERROR_ALERT "Failed to read \"%s\" aspect in an entity-describing dictionary.\n", aspect_readers[j].aspect_name);
                    exit(EXIT_FAILURE);
                }
                printf("Succeeded in reading a \"%s\" aspect.\n", aspect_readers[j].aspect_name);
            }

            ////////////////////////////////////////////////////////////////////////////////
            ////// close the dictionaries
            
        }
    }
}





