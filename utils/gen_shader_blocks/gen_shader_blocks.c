#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include "gen_shader_blocks.h"
#include "tokens.yy.h"


char* token_name(int token)
{
#define tok(TOK) case TOK:\
    return #TOK; break;
    switch(token) {
        tok(BLOCK);
        tok(STRUCT);
        tok(LEFTBRACE);
        tok(RIGHTBRACE);
        tok(LEFTBRACKET);
        tok(RIGHTBRACKET);
        tok(SEMICOLON);
        tok(IDENTIFIER);
        tok(HASHDEFINE);
    }
    fprintf(stderr, "Invalid token given to convert to string.\n");
    exit(EXIT_FAILURE);
}

int new_symbol(char *string)
{
    #define table_check() if (symbol_table == NULL) { fprintf(stderr, "ERROR: Could not allocate memory for symbol table.\n"); exit(EXIT_FAILURE); }
    if (symbol_table == NULL) {
        symbol_table_size = SYMBOL_TABLE_START_SIZE;
        symbol_table_position = 0;
        symbol_table = (char *) calloc(1, symbol_table_size * sizeof(char));
        table_check();
    } else if (symbol_table_position + strlen(string) >= symbol_table_size) {
        symbol_table_size *= 2;
        symbol_table = (char *) realloc(symbol_table, symbol_table_size * sizeof(char));
        table_check();
    }
    int entry = symbol_table_position;
    strcpy(symbol_table + symbol_table_position, string);
    symbol_table_position += strlen(string) + 1;
    /* print_symbol_table(); */
    return entry;
}
void print_symbol_table(void)
{
    printf("------------\n");
    printf("SYMBOL TABLE\n");
    printf("------------\n");
    for (int i = 0; i < symbol_table_position; i++) {
        if (symbol_table[i] == '\0') putchar('\n');
        else putchar(symbol_table[i]);
    }
    printf("------------\n");
}
char *symbol(int entry)
{
    return symbol_table + entry;
}

void yyerror(char *errmsg)
{
    printf("parsing ERROR!!!! errmsg: %s\n", errmsg);
}

Block g_block;
StructDefinition g_struct;
void new_block(void)
{
    memset(&g_block, 0, sizeof(Block));
    g_block.name = -1; // this indicates that the block has not yet been filled.
}
void finish_block(int block_name)
{
    g_block.name = block_name;
#if 0
    printf("BLOCK FINISHED\n");
    printf("name: %s\n", symbol(g_block.name));
    printf("num_entries: %d\n", g_block.num_entries);
    for (int i = 0; i < g_block.num_entries; i++) {
        printf("entry: [%s] [%s]\n", symbol(g_block.entries[i].type), symbol(g_block.entries[i].the_rest));
    }
    printf("num_struct_definitions: %d\n", g_block.num_struct_definitions);
    for (int i = 0; i < g_block.num_struct_definitions; i++) {
        printf("struct: [%s]\n", symbol(g_block.struct_definitions[i].name));
        printf("\tstruct's num_entries: %d\n", g_block.struct_definitions[i].num_entries);
        for (int j = 0; j < g_block.struct_definitions[i].num_entries; j++) {
            printf("\tentry: [%s] [%s]\n", symbol(g_block.struct_definitions[i].entries[j].type), symbol(g_block.struct_definitions[i].entries[j].the_rest));
        }
    }
    printf("num_hash_defines: %d\n", g_block.num_hash_defines);
    for (int i = 0; i < g_block.num_hash_defines; i++) {
        printf("hash_define: [%s]\n", symbol(g_block.hash_defines[i]));
    }
#endif
}
void block_add_entry(Entry entry)
{
    if (g_block.num_entries >= MAX_ENTRIES) { fprintf(stderr, "ERROR: Too many entries in block.\n"); exit(EXIT_FAILURE); }
    g_block.entries[g_block.num_entries ++] = entry;
}
void block_add_struct(int name)
{
    if (g_block.num_struct_definitions >= MAX_STRUCT_DEFINITIONS) { fprintf(stderr, "ERROR: Too many structs defined in block.\n"); exit(EXIT_FAILURE); }
    g_struct.name = name;
    // Calculate the !!! std140 !!! size of the struct.
    g_struct.std140_size = std140_layout(&g_block, g_struct.entries, g_struct.num_entries);
    g_block.struct_definitions[g_block.num_struct_definitions ++] = g_struct;
}
void block_add_hash_define(HashDefinition hash_definition)
{
    if (g_block.num_hash_defines >= MAX_HASH_DEFINES) { fprintf(stderr, "ERROR: Too many #defines in block.\n"); exit(EXIT_FAILURE); }
    g_block.hash_defines[g_block.num_hash_defines ++] = hash_definition;
}


size_t std140_layout(Block *block, Entry *entries, int num_entries)
{
    // entries and num_entries are given because this will be used for std140ing both block entries and struct entries.
    // Fills the entries with std140 layout information (size, alignment, offset, etc.)
/*--------------------------------------------------------------------------------
    OpenGL programming guide 8th edition
         Appendix I, page 886, The std140 Layout Rules
----------------------------------------
Scalar bool, int, uint, float and double
----------------------------------------
    Both the size and alignment are the size of the scalar in basic machine types (e.g., sizeof(GLfloat)).
-----------------------------------
Two-component vectors (e.g., ivec2)
-----------------------------------
    Both the size and alignment are twice the size of the underlying scalar type.
----------------------------------------------------------------------------
Three-component vectors (e.g., vec3) and Four-component vectors (e.g., vec4)
----------------------------------------------------------------------------
    Both the size and alignment are four times the size of the underlying scalar type.
------------------------------
An array of scalars or vectors
------------------------------
    The size of each element in the array will be the size of the element type, rounded up to a multiple of the size of a
    vec4. This is also the array’s alignment.  The array’s size will be this rounded-up element’s size times the number of
    elements in the array.
---------------------------------------------------------------------------------------
A column-major matrix or an array of column-major matrices of size C columns and R rows
---------------------------------------------------------------------------------------
    Same layout as an array of N vectors each with R components, where N is the total number of columns present.
------------------------------------------------------------------------------
A row-major matrix or an array of row-major matrices with R rows and C columns
------------------------------------------------------------------------------
    Same layout as an array of N vectors each with C components, where N is the total number of rows present.
--------------------------------------------------------
A single-structure definition, or an array of structures
--------------------------------------------------------
    Structure alignment will be the alignment for the biggest structure member, according to the previous
    rules, rounded up to a multiple of the size of a vec4. Each structure will start on this alignment, and its size will be the
    space needed by its members, according to the previous rules, rounded up to a multiple of the structure alignment.
--------------------------------------------------------------------------------*/
#define up(SIZE,ALIGNMENT)\
    ( SIZE ) % ( ALIGNMENT ) == 0 ? ( SIZE ) : ((( SIZE ) + ( ALIGNMENT )) - (( SIZE ) % ( ALIGNMENT )))
    // ---Maybe could do some bit trick, but there is a problem with the case when it is already aligned, and it is bumped up by one alignment.
#define up_vec4(SIZE)\
    up(SIZE,16)
#define type_error(TYPE)\
    {\
        fprintf(stderr, "ERROR: Unkown type \"%s\".\n", symbol(( TYPE )));\
        exit(EXIT_FAILURE);\
    }
    size_t offset = 0;
    // Unravel the entries into their std140 layout.
    for (int i = 0; i < num_entries; i++) {
        size_t alignment;
        size_t size;
        size_t C_type_size;
        if (entries[i].is_struct) {
            /*
A single-structure definition, or an array of structures
--------------------------------------------------------
    Structure alignment will be the alignment for the biggest structure member, according to the previous
    rules, rounded up to a multiple of the size of a vec4. Each structure will start on this alignment, and its size will be the
    space needed by its members, according to the previous rules, rounded up to a multiple of the structure alignment.
            */
            StructDefinition *struct_def = &block->struct_definitions[entries[i].struct_number];
            // Align the structure/s to the size of the biggest structure member, rounded up to a multiple of the size of a vec4 (16 bytes).
            size_t max_size = 0;
            Entry biggest_structure_member = struct_def->entries[0]; // !!! Struct better be non-empty.
            for (int j = 0; j < struct_def->num_entries; j++) {
                if (struct_def->entries[i].type_size > max_size) {
                    max_size = struct_def->entries[i].type_size;
                    biggest_structure_member = struct_def->entries[i];
                }
            }
            alignment = up_vec4(biggest_structure_member.std140_alignment);

            if (entries[i].is_array) { // array of structures
                size = up(struct_def->std140_size, alignment) * entries[i].array_length;
            } else {
                size = up(struct_def->std140_size, alignment);
            }
            C_type_size = size;
        } else if (entries[i].is_array) {
            /*
An array of scalars or vectors
------------------------------
    The size of each element in the array will be the size of the element type, rounded up to a multiple of the size of a
    vec4. This is also the array’s alignment.  The array’s size will be this rounded-up element’s size times the number of
    elements in the array.
            */
            size_t element_size = glsl_type_size(symbol(entries[i].type));
            if (element_size == 0) type_error(entries[i].type);
            alignment = up_vec4(element_size);
            size = up_vec4(element_size) * entries[i].array_length;
            C_type_size = size;
            
        } else { // singleton built-in type: scalar, vector, or matrix.
            #define is(NAME)\
                ( strcmp(symbol(entries[i].type), ( NAME )) == 0 )
            if (is("bool") || is("int") || is("uint") || is("float") || is("double") || is("vec2") || is("ivec2") || is("vec4") || is("ivec4")) {
                C_type_size = glsl_type_size(symbol(entries[i].type));
                size = C_type_size;
                alignment = size;
            } else if (is("vec3") || is("ivec3")) {
                // *vec3 alignment to the size of the *vec4.
                // This is done with string manipulation so that the type sizes are handled in the glsl_type_size function.
                C_type_size = glsl_type_size(symbol(entries[i].type));
                size = up_vec4(C_type_size);
                alignment = size;
            } else if (is("mat4x4")) {
                C_type_size = 64;
                size = 64;
                alignment = 16;
            } else if (is("mat3x3")) {
                C_type_size = 48; // sizeof(float) * sizeof(vec3_plus_1) * 3
                size = 48;
                alignment = 16;
            } else {
                fprintf(stderr, "ERROR: Unrecognized type \"%s\".\n", symbol(entries[i].type));
                exit(EXIT_FAILURE);
            }
            #undef is
        }
        /* printf("type %s size %zu\n", symbol(entries[i].type), size); */
        printf("%s\n", symbol(entries[i].type));
        printf("---offset: %zu\n", offset);
        printf("---up to alignment: %zu\n", alignment);
        offset = up(offset, alignment);
        entries[i].type_size = C_type_size;
        entries[i].std140_size = size;
        entries[i].std140_offset = offset;
        entries[i].std140_alignment = alignment;
        printf("------offset: %zu\n", offset);
        printf("------plus size: %zu\n", size);
        offset += size;
        printf("----------offset: %zu\n", offset);
    }
    return offset; //the offset is placed after the end, so is the size.
#undef up
#undef up_vec4
#undef type_error
}

size_t glsl_type_size(char *type)
{
    // This returns 0 when the type doesn't match, so this is also used to test if a glsl-type is accounted for.
#define size(NAME,SIZE)\
    if (strcmp(type, ( NAME )) == 0) return ( SIZE );
    size("bool", 1);
    size("float", 4);
    size("double", 8);
    size("int", 4);
    size("uint", 4);
    size("vec2", 8);
    size("vec3", 12);
    size("vec4", 16);
    size("ivec2", 8);
    size("ivec3", 12);
    size("ivec4", 16);
    size("mat4x4", 64);
    size("mat3x3", 48);
    return 0;
#undef size
}

Entry new_entry(int type, int the_rest)
{
    Entry entry;
    entry.type = type;

    // Check if this is a built-in glsl type which is accounted for.
    size_t type_size = glsl_type_size(symbol(type));
    if (type_size == 0) {
        // Otherwise, check if it is a struct type of a struct that has been declared previously (the order matters).
        int i;
        for (i = 0; i < g_block.num_struct_definitions; i++) {
            if (strcmp(symbol(type), symbol(g_block.struct_definitions[i].name)) == 0) {
                entry.is_struct = true;
                entry.struct_number = i;
                entry.type_size = g_block.struct_definitions[i].std140_size;
                break;
            }
        }
        if (i == g_block.num_struct_definitions) {
            fprintf(stderr, "ERROR: Type \"%s\" is not a built-in glsl type or a struct that has been declared previously in the block.\n", symbol(type));
            exit(EXIT_FAILURE);
        }
    } else {
        entry.is_struct = false;
        entry.type_size = type_size;
    }
    entry.the_rest = the_rest;
    return entry;
}
void new_struct(void)
{
    memset(&g_struct, 0, sizeof(StructDefinition));
}
void struct_add_entry(Entry entry)
{
    if (g_struct.num_entries >= MAX_ENTRIES) { fprintf(stderr, "ERROR: Too many entries in struct.\n"); exit(EXIT_FAILURE); }
    g_struct.entries[g_struct.num_entries ++] = entry;
}


// Coroutine interface to the parser. It returns a block one by one.
bool get_block(Block *block)
{
    if (yyparse() != 0) {
        return false;
    }
    // Generate std140 offsets for the block.
    std140_layout(&g_block, g_block.entries, g_block.num_entries);
    memcpy(block, &g_block, sizeof(Block));
    return true;
}

#define GENERATED_CODE_COMMENT "/*--------------------------------------------------------------------------------\nThis code was generated with the gen_shader_blocks utility, for synchronizing\ndefinitions between glsl and C.\n--------------------------------------------------------------------------------*/\n"
static void glsl_generate_entries(FILE *file, Entry *entries, int num_entries)
{
    for (int i = 0; i < num_entries; i++) {
        if (entries[i].is_array) {
            fprintf(file, "    %s %s[%d];\n", symbol(entries[i].type), symbol(entries[i].the_rest), entries[i].array_length);
        } else {
            fprintf(file, "    %s %s;\n", symbol(entries[i].type), symbol(entries[i].the_rest));
        }
    }
}
void generate_block_glsl(FILE *file, Block block)
{
    printf("Generating glsl block ...\n");
    fputs(GENERATED_CODE_COMMENT, file);
    for (int i = 0; i < block.num_hash_defines; i++) {
        fprintf(file, "#define %s %d\n", symbol(block.hash_defines[i].identifier), block.hash_defines[i].value);
    }
    if (block.num_hash_defines != 0) fprintf(file, "\n");
    for (int i = 0; i < block.num_struct_definitions; i++) {
        fprintf(file, "struct %s {\n", symbol(block.struct_definitions[i].name));
        glsl_generate_entries(file, block.struct_definitions[i].entries, block.struct_definitions[i].num_entries);
        fprintf(file, "};\n");
    }
    if (block.num_struct_definitions != 0) fprintf(file, "\n");
    fprintf(file, "layout (std140) uniform %s {\n", symbol(block.name));
    glsl_generate_entries(file, block.entries, block.num_entries);
    fprintf(file, "};\n");
}

// The generation of the internal entries in the struct for the block and for the other structs is the same,
// so this function is separated.
static void C_generate_entries(FILE *file, Block *block, Entry *entries, int num_entries)
{
    for (int i = 0; i < num_entries; i++) {
        // ---struct renaming
        size_t type_size = entries[i].type_size;
        size_t std140_alignment = entries[i].std140_alignment;
        size_t std140_offset = entries[i].std140_offset;
        if (i > 0) {
            // std140 padding
            size_t prev_type_size = entries[i-1].type_size;
            size_t prev_std140_offset = entries[i-1].std140_offset;
            // This calculates the amount of byte padding that needs to be sandwhiched from entry i-1 to i
            // to account for the lack of reaching the wanted offset.
            size_t pad = std140_offset - prev_std140_offset - prev_type_size;
            if (pad != 0) fprintf(file, "    char ___std140_pad%d[%zu];\n", i, pad);
        }
        if (entries[i].is_array) {
            if (entries[i].is_struct) {
                fprintf(file, "    struct ShaderBlockStruct_%s_%s %s[%d];", symbol(block->name), symbol(entries[i].type), symbol(entries[i].the_rest), entries[i].array_length);
            } else {
                fprintf(file, "    %s %s[%d];", symbol(entries[i].type), symbol(entries[i].the_rest), entries[i].array_length);
            }
        } else {
            if (entries[i].is_struct) {
                fprintf(file, "    struct ShaderBlockStruct_%s_%s %s;", symbol(block->name), symbol(entries[i].type), symbol(entries[i].the_rest));
            } else {
                fprintf(file, "    %s %s;", symbol(entries[i].type), symbol(entries[i].the_rest));
            }
        } 
        fprintf(file, "    //offset: %zu, alignment: %zu, C_type_size: %zu\n", std140_offset, std140_alignment, type_size);

    }
}
void generate_block_C(FILE *file, Block block)
{
/*-[C]----------------------------------------------------------------------------
--include/shader_blocks/Standard3D.h
// This is generated code!! (or some comment like this)
ShaderBlockID ShaderBlockID_Standard3D;
typedef struct ShaderBlock_Standard3D_s {
    mat4x4 mvp_matrix;
} ShaderBlock_Standard3D;

--include/shader_blocks/StandardLoopWindow.h
ShaderBlockID ShaderBlockID_StandardLoopWindow;
typedef struct ShaderBlock_StandardLoopWindow_s {
    // padding bytes would be in here if neccessary.
    float aspect_ratio;
    float time;
} ShaderBlock_StandardLoopWindow;

--include/shader_blocks/DirectionalLights.h
typedef struct ShaderBlockStruct_DirectionalLight_s {
    bool is_active;
    char _pad1[15];
    vec3 direction;
    char _pad2[4];
    vec4 color;
} ShaderBlockStruct_DirectionalLight;

--include/shader_blocks/Standard3D.h
ShaderBlockID ShaderBlockID_DirectionalLights;
#define MAX_NUM_DIRECTIONAL_LIGHTS 32
typedef struct ShaderBlock_DirectionalLights_s {
    ShaderBlockStruct_DirectionalLight directional_light[MAX_NUM_DIRECTIONAL_LIGHTS];
} ShaderBlock_DirectionalLights;
--------------------------------------------------------------------------------*/
#define PUT_UPPERCASE_NAME() for (int i = 0; i < strlen(symbol(block.name)); i++) fputc(toupper(symbol(block.name)[i]), file);
    printf("Generating C block ...\n");
    fputs(GENERATED_CODE_COMMENT, file);
    fprintf(file, "#ifndef SHADER_BLOCK_HEADER_DEFINED_"); PUT_UPPERCASE_NAME(); fprintf(file, "\n");
    fprintf(file, "#define SHADER_BLOCK_HEADER_DEFINED_"); PUT_UPPERCASE_NAME(); fprintf(file, "\n");
    for (int i = 0; i < block.num_hash_defines; i++) {
        fprintf(file, "#define %s %d\n", symbol(block.hash_defines[i].identifier), block.hash_defines[i].value);
    }
    if (block.num_hash_defines != 0) fprintf(file, "\n");
    for (int i = 0; i < block.num_struct_definitions; i++) {
        fprintf(file, "struct ShaderBlockStruct_%s_%s { //size: %zu\n", symbol(block.name), symbol(block.struct_definitions[i].name), block.struct_definitions[i].std140_size);
        C_generate_entries(file, &block, block.struct_definitions[i].entries, block.struct_definitions[i].num_entries);
        fprintf(file, "};\n");
    }
    if (block.num_struct_definitions != 0) fprintf(file, "\n");
    fprintf(file, "ShaderBlockID ShaderBlockID_%s;\n", symbol(block.name));
    fprintf(file, "typedef struct ShaderBlock_%s_s {\n", symbol(block.name));
    C_generate_entries(file, &block, block.entries, block.num_entries);
    fprintf(file, "} ShaderBlock_%s;\n", symbol(block.name));
    fprintf(file, "\n#endif // SHADER_BLOCK_HEADER_DEFINED_"); PUT_UPPERCASE_NAME(); fprintf(file, "\n");
#undef PUT_UPPERCASE_NAME
}

int main(int argc, char **argv)
{
#if 1
    if (argc != 6) {
        fprintf(stderr, "give good args\n");
        exit(EXIT_FAILURE);
    }
    char *shader_blocks_filename = argv[1];
    char *glsl_directory;
    char *C_directory;
    for (int i = 2; i <= 4; i += 2) {
        if (strcmp(argv[i], "-c") == 0) C_directory = argv[i+1];
        else if (strcmp(argv[i], "-g") == 0) glsl_directory = argv[i+1];
    }
    FILE *file = fopen(shader_blocks_filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open shader-blocks file \"%s\".\n", shader_blocks_filename);
        exit(EXIT_FAILURE);
    }
    yyin = file;

    while (1) {
        new_block(); // wipe the block. this is just so it is known whether or not a get_block call really did create a block.
        Block block;
        if (!get_block(&block)) {
            // A false return value from get_block is an _error_, not an indicator that the block stream has finished.
            // When a block stream is finished, the block simply isn't interacted with.
            fprintf(stderr, "ERROR: Could not understand a block. This block and no subsequent blocks have been processed.\n");
            exit(EXIT_FAILURE);
        }
        if (block.name == -1) {
            // this signifies that the block hasn't been filled, so the block stream is finished (with no error).
            break;
        }
        // Form the names of the two files, the C and glsl files, which are going to contain generated code.
        const int buf_size = 2048;
        char glsl_filename[buf_size];
        sprintf(glsl_filename, "%s/%s.glh", glsl_directory, symbol(block.name));
        char C_filename[buf_size];
        sprintf(C_filename, "%s/%s.h", C_directory, symbol(block.name));

        FILE *glsl_file = fopen(glsl_filename, "w+");
        if (glsl_file == NULL) {
            fprintf(stderr, "Failed to open file \"%s\" to generate glsl shader-block code.\n", glsl_filename);
            exit(EXIT_FAILURE);
        }
        FILE *C_file = fopen(C_filename, "w+");
        if (C_file == NULL) {
            fprintf(stderr, "Failed to open file \"%s\" to generate C shader-block code.\n", C_filename);
            exit(EXIT_FAILURE);
        }
        generate_block_glsl(glsl_file, block);
        generate_block_C(C_file, block);

        fclose(glsl_file);
        fclose(C_file);
    }


    /* // test the lexer */
    /* int token; */
    /* /1* trace_lex = true; *1/ */
    /* while ((token = yylex()) != LEX_DONE) { */
    /*     if (token == LEX_ERROR) { */
    /*         fprintf(stderr, "ERROR: LEX FAILED.\n"); */
    /*         exit(EXIT_FAILURE); */
    /*     } */
    /*     printf("Token: %s\n", token_name(token)); */
    /* } */
    /* printf("Lex success!\n"); */
    /* fseek(file, 0, SEEK_SET); */

    /* /1* trace_lex = false; *1/ */
    /* printf("PARSING\n"); */
    /* printf("=======\n"); */
    /* if (yyparse() != 0) { */
    /*     printf("Parsing error.\n"); */
    /* } */
    /* printf("Completed parsing,\n"); */
#endif
}
