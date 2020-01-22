/*--------------------------------------------------------------------------------
    Resources, resource management, and resource loading module.
--------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "helper_definitions.h"
#include "resources.h"
#include "data_dictionary.h"

// The global resource dictionary is a data-dictionary intended to be set by the application and used when searching for resources.
// This dictionary is the "root", and each resource has a GUID, naturally its path from the root, with no /-prefix.
DataDictionary *g_resource_dictionary = NULL;



