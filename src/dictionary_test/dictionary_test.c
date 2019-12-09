/*
PROJECT_LIBS:
    + dictionary
*/
#include <stdio.h>
#include "helper_definitions.h"
#include "dictionary.h"

static void test_get(Dictionary *dict, char *key, char *buf, int buf_size)
{
    printf("Querying for \"%s\": ", key);
    if (!dict_get(dict, key, buf, buf_size)) printf("Not found!\n");
    else printf("got \"%s\"\n", buf);
}

int main(int argc, char **argv)
{
    argcheck(2);
    fopen_handle(file, argv[1], "r");

    Dictionary *dict = dictionary_read(file);
    if (dict == NULL) {
        fprintf(stderr, "Couldn't read the dictioanry.\n");
        exit(EXIT_FAILURE);
    }
    print_dictionary(dict);

    const int buf_size = 128;
    char buf[buf_size];
    test_get(dict, "vertex_format", buf, buf_size);
    test_get(dict, "file_type", buf, buf_size);

    destroy_dictionary(dict);
}
