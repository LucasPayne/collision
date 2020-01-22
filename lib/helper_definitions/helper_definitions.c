#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "helper_definitions.h"

uint32_t hash_crc32(char *string)
{
    // This is not crc32 ...
    uint32_t hash = 0;
    int len = strlen(string);
    for (int i = 0; i < len; i++) {
        int n = 1;
        for (int j = 0; j < i; j++) {
            n *= string[i];
        }
        hash += n;
    }
    return hash;
}
