/*
The point of this is so that the standard binary format for a compiled asset is as close to
program-ready as possible before any post-conditioning.

So, this binary format is some minimal metadata (or implicit metadata due to how it is interpreted, maybe magic numbers as well),
which allows the binary file to be loaded into memory as structured data.

With the resource system, a MeshHandle is wrapped around with a ResourceHandle. This has a load behaviour which
loads from the standard binary format for Meshes, which is almost the same as it will be in memory. The pointer fix-ups are
done and maybe separate parts of it are put in different parts of memory.
Post-conditioning is done, which passes the Mesh to a vram uploader, which fills a MeshHandle, which is then
the loaded resource. The binary data read in initially is then freed.

The compiled mesh format on disk is then really an implicitly defined format due to the structure of a mesh in the code. It should
be easy to detect changes when regenerating serialization information and recompile assets. It is very close to the form the mesh
will take in memory on whatever system the asset was compiled for.
 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


typedef uint32_t PODSType;

enum PODSAddressModes {
    PODS_None,
    PODS_Struct, // Address of a struct
    PODS_String, // Null-termination of the string entry will be taken into account in the serialization/deserialization.
    PODS_Array
};
enum PODSCountTypes {
    PODS_uint8,
    PODS_uint16,
    PODS_uint32,
    PODS_uint64,
};
typedef struct PODSAddressInfo_s {
    uint8_t mode;
    uint32_t address_offset;
    // Struct mode
    uint32_t struct_type;
    // Array mode
    uint8_t count_type;
    uint32_t array_entry_size;
    uint32_t count_offset;
} PODSAddressInfo;

/* A PODS type ID is used as the index to get the offset into a global pods address info array,
 * and the number of address infos.
 */
static uint32_t g_pods_info_offsets[64];
static uint32_t g_pods_info_num_addresses[64];
static uint32_t g_pods_info_type_sizes[64];
static PODSAddressInfo *g_pods_info;

typedef struct SomeStruct_s {
    int a;
    char *name;
    uint8_t b;
} SomeStruct;

#define NUM_VERTEX_ATTRIBUTE_TYPES 10
typedef struct MeshData_s {
    uint32_t format;
    uint32_t num_vertices;
    SomeStruct *some_struct;
    void *attribute_data[NUM_VERTEX_ATTRIBUTE_TYPES];
    uint32_t *triangle_indices;
} MeshData;

// How does the serializer know how large the dynamic arrays are? If serialization is going to mostly be done with generated code,
// it might be fine to associate to every dynamic array entry of a serializable structure a function that retrieves the size of the array.
// Because this needs to actually be entered, a file for configuring serialization could be used.
//
// A utility could go over the C source and generate basic metadata for structures (names, assign them ids, offsets and types and sizes of entries).
// Serializing a structure expands a macro to a global ___StructureName___PODS_ID which hooks it up to the generated serialization information for this structure.
// By default, the structure is serialized as is, so no interpretation of pointers, dynamic arrays, pointers to other structures. These are introduced explicitly
// or if it makes sense, automatically (for structure pointers that makes sense, but dynamic arrays need some way to know the size. This could be stored in the struct, it could
// be null terminated in some way, it could be a multiple of some type size, dependent on some configuration of the struct, ... . Serializer can't know, this needs user
// configuration and direction.)
//
// One restrictive option for serializable dynamic arrays is to have an array-type address-info-structure for a structure contain an offset and type for an unsigned int
// which will hold the array size. This is too restrictive.
static size_t PODS_get_size_MeshData_triangle_indices(void *struct_ptr)
{
    MeshData *mesh_data = (MeshData *) struct_ptr;
    return sizeof(uint32_t) * 3 * mesh_data->num_vertices;
}


#define PODS_serialize(FILE,PODS_TYPE_NAME,PODS_STRUCT_PTR)\
    ___PODS_serialize(( FILE ), ( PODS_TYPE_NAME ## _PODS_ID ), (void *) ( PODS_STRUCT_PTR ), 0)
uint32_t ___PODS_serialize(FILE *file, PODSType pods_type, void *struct_ptr, uint32_t file_pos)
{
    uint32_t size = g_pods_info_type_sizes[pods_type];
    uint32_t offset = g_pods_info_offsets[pods_type];
    uint32_t num_addresses = g_pods_info_num_addresses[pods_type];

    // --- write over addresses with file offsets without modifying struct (when writing to file)
    if (fwrite(struct_ptr, size, 1, file) != 1) goto serialize_error;
    file_pos += size;

    for (int i = offset; i < offset + num_addresses; i++) {
        void *pointed_to = *((void **) (struct_ptr + g_pods_info[i].address_offset));
        if (pointed_to == NULL) continue;

        switch(g_pods_info[i].mode) {
        case PODS_Struct:
            file_pos = ___PODS_serialize(file, g_pods_info[i].struct_type, pointed_to, file_pos);

            break;
        case PODS_String:

            break;
        case PODS_Array:
            uint64_t count;
            switch(g_pods_info[i].count_type) {
                case PODS_uint8:  count = (uint64_t) *((uint8_t *)  (struct_ptr + g_pods_info[i].count_offset)); break;
                case PODS_uint16: count = (uint64_t) *((uint16_t *) (struct_ptr + g_pods_info[i].count_offset)); break;
                case PODS_uint32: count = (uint64_t) *((uint32_t *) (struct_ptr + g_pods_info[i].count_offset)); break;
                case PODS_uint64: count = (uint64_t) *((uint64_t *) (struct_ptr + g_pods_info[i].count_offset)); break;
            }
            if (fwrite(pointed_to, g_pods_info[i].array_entry_size, count, file) != count) goto serialize_error;
            file_pos += g_pods_info[i].array_entry_size * count;
            break;
        }
    }
serialize_error:
    fprintf(stderr, ERROR_ALERT "Error when serializing PODS.\n");
    exit(EXIT_FAILURE);
}





int main(int argc, char **argv)
{
    if (argc != 2) exit(EXIT_FAILURE);
    char *filename = argv[1];

    FILE *file = fopen(filename, "w+");
    if (file == NULL) {
        fprintf(stderr, ERROR_ALERT "Could not open or create file for PODS serialization.\n");
        exit(EXIT_FAILURE);
    }

    PODS_add_type(SomeStruct, 1);
    PODS_set_address_info_array(SomeStruct, 0, sizeof(int), PODS_uint8, sizeof(int) + sizeof(uint8_t), sizeof(char));

    PODSType MeshData_type = PODS_add_type(MeshData, 3);
    PODS_set_address_info_struct(mesh_data_type, 0, 2 * sizeof(uint32_t), SomeStruct);
    /* PODS_set_address_info_array(MeshData, 1, sizeof(uint32_t), PODS_uint32, 2 * sizeof(uint32_t) + sizeof(SomeStruct *), sizeof(void */ 

    PODS_set_address_info_array(MeshData_type, 1, 




typedef struct SomeStruct_s {
    int a;
    uint8_t name_length;
    char *name;
    uint8_t b;
} SomeStruct;

typedef struct MeshData_s {
    uint32_t format;
    uint32_t num_vertices;
    SomeStruct *some_struct;
    void *attribute_data[NUM_VERTEX_ATTRIBUTE_TYPES];
    uint32_t *triangle_indices;
} MeshData;


    PODS_serialize(file, MeshData);
}
