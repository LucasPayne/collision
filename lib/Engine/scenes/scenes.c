/*--------------------------------------------------------------------------------
    Scenes
--------------------------------------------------------------------------------*/
#include "Engine.h"

typedef bool (*AspectReader)(DataDictionary *, void *);
#define new_reader(NAME) bool read_aspect_ ## NAME(DataDictionary *aspect_dd, void *data)
new_reader(Transform) {
    vec3 position;
    if (!dd_get(aspect_dd, "position", "vec3", &position)) return false;
    printf("got position: %f %f %f\n", position.vals[0], position.vals[1], position.vals[2]);
    vec3 rotation; // euler angles
    if (!dd_get(aspect_dd, "rotation", "vec3", &rotation)) return false;
    Transform *transform = (Transform *) data;
    transform->x = position.vals[0];
    transform->y = position.vals[1];
    transform->z = position.vals[2];
    transform->theta_x = rotation.vals[0];
    transform->theta_y = rotation.vals[1];
    transform->theta_z = rotation.vals[2];
    float scale;
    if (!dd_get(aspect_dd, "scale", "float", &scale)) return false;
    transform->scale = scale;
    return true;
}
new_reader(Body) {
    Body *body = (Body *) data;
    body->visible = true;
    Material *mat = oneoff_resource(Material, body->material);
    char *geometry_path;
    if (!dd_get(aspect_dd, "geometry", "string", &geometry_path)) return false;
    body->geometry = new_resource_handle(Geometry, geometry_path);
    char *material_path;
    if (!dd_get(aspect_dd, "material", "string", &material_path)) return false;
    body->material = new_resource_handle(Material, material_path);
    bool is_ground; // "ground" doesn't cast shadows, so shadow maps can have increased resolution up to the non-ground scene.
    if (!dd_get(aspect_dd, "is_ground", "bool", &is_ground)) return false;
    body->is_ground = is_ground;
    
    //----dummy for now
    // Material *mat = oneoff_resource(Material, body->material);
    // mat->material_type = new_resource_handle(MaterialType, "Materials/red");
    // body->geometry = new_resource_handle(Geometry, "Models/quad");

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
        fprintf(stderr, ERROR_ALERT "Could not open scene dictionary from path \"%s\".\n", path);
	exit(EXIT_FAILURE);
    }

    DD **gameobject_dictionaries;
    int num_gameobjects = dd_scan(scene_dictionary, &gameobject_dictionaries, "GameObject");
    printf("num_gameobjects: %d\n", num_gameobjects);
    for (int i = 0; i < num_gameobjects; i++) {
        bool testing_value;
        if (dd_get(gameobject_dictionaries[i], "testing_value", "bool", &testing_value)) printf("testing_value: %s\n", testing_value ? "true":"false");

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
                printf("Reading aspect ...\n");
	        if (!aspect_readers[j].read_aspect(aspect_dictionaries[k], data)) {
                    fprintf(stderr, ERROR_ALERT "Failed to read \"%s\" aspect in an entity-describing dictionary.\n", aspect_readers[j].aspect_name);
                    exit(EXIT_FAILURE);
                }
                printf("Succeeded in reading a \"%s\" aspect.\n", aspect_readers[j].aspect_name);
                if (aspect_type == Transform_TYPE_ID) { printf("%f %f %f\n", ((Transform *) data)->x, ((Transform *) data)->y, ((Transform *) data)->z); }
            }

            ////////////////////////////////////////////////////////////////////////////////
            ////// close the dictionaries
            
        }
    }
}





