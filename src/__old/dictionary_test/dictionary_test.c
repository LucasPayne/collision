/*
PROJECT_LIBS:
    + dictionary
    + resources
*/
#include <stdio.h>
#include "helper_definitions.h"
#include "dictionary.h"
#include "resources.h"

static void test_get(Dictionary *dict, char *key, char *buf, int buf_size)
{
    printf("Querying for \"%s\": ", key);
    if (!dict_get(dict, key, buf, buf_size)) printf("Not found!\n");
    else printf("got \"%s\"\n", buf);
}

ResourceType Thing_RTID;
typedef struct /* Resource */ Thing_s {
    int val;
} Thing;
static void *Thing_load(char *path)
{
    // Won't have this many fatal error messages, but it is for testing.
    printf("Path: %s\n", path);
    FILE *file = resource_file_open(path, ".Thing", "r");
    if (file == NULL) {
        fprintf(stderr, "COULD NOT OPEN!!!!\n");
        exit(EXIT_FAILURE);
    }
    Thing *thing = (Thing *) calloc(1, sizeof(Thing));
    Dictionary *dict = dictionary_read(file);
    if (dict == NULL) {
        fprintf(stderr, "Couldn't read the dictionary.\n");
        exit(EXIT_FAILURE);
    }
    print_dictionary(dict);
    const int buf_size = 128;
    char buf[buf_size];
    if (!dict_get(dict, "val", buf, buf_size)) {
        fprintf(stderr, "NO val ENTRY!!!!\n");
        exit(EXIT_FAILURE);
    }
    sscanf(buf, "%d", &thing->val);
    return thing;
}

int main(int argc, char **argv)
{
    argcheck(2);
    fopen_handle(file, argv[1], "r");

    resource_path_add("Stuff", "/home/lucas/code/collision/src/dictionary_test/stuff");
    resource_path_add("MoreStuff", "/home/lucas/code/collision/src/dictionary_test/more_stuff");
    print_resource_path();
    add_resource_type(Thing);
    print_resource_types();
    {
        ResourceHandle thing = new_resource_handle(Thing, "Stuff/example");
        resource_data(Thing, thing);
        printf("Got a resource of type Thing:\n");
        printf(".val = %d\n", resource_data(Thing, thing)->val);
    }
    {
        ResourceHandle thing = new_resource_handle(Thing, "MoreStuff/another");
        resource_data(Thing, thing);
        printf("Got a resource of type Thing:\n");
        printf(".val = %d\n", resource_data(Thing, thing)->val);
    }

#if 0
    Dictionary *dict = dictionary_read(file);
    if (dict == NULL) {
        fprintf(stderr, "Couldn't read the dictionary.\n");
        exit(EXIT_FAILURE);
    }
    print_dictionary(dict);
    const int buf_size = 128;
    char buf[buf_size];
    test_get(dict, "vertex_format", buf, buf_size);
    test_get(dict, "file_type", buf, buf_size);
    destroy_dictionary(dict);
#endif
}
