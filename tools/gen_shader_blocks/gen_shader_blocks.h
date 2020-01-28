#ifndef HEADER_DEFINED_GEN_SHADER_BLOCKS
#define HEADER_DEFINED_GEN_SHADER_BLOCKS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Uncomment this for debugging.
#define DEBUG
#ifdef DEBUG
#define debugp(STRING) { printf("%s\n", ( STRING )); }
#undef DEBUG
#else
#define debugp(STRING) { }
#endif

#define LEX_DONE -1 // Returned by yylex to indicate that it has finished successfully.
#define LEX_ERROR -2 // Returned by yylex to indicate that it has not been able to lex the input.
char* token_name(int token);

// The "symbol table" here is really just so there is a central place to deallocate strings.
// A symbol table entry is the index of the null-terminated string in this array.
//--- This "symbol table" does not look for the same instances of an identifier (could be done with hashing).
// This might be wanted for correlating the same struct name.
static char *symbol_table = NULL;
static size_t symbol_table_size;
static int symbol_table_position = 0;
#define SYMBOL_TABLE_START_SIZE 3000
int new_symbol(char *string);
void print_symbol_table(void);
char *symbol(int entry);

extern FILE *yyin;
int yylex(void);
void yyerror(char *errmsg);

// Intermediate representation, a list of "blocks".
#define MAX_HASH_DEFINES 256
#define MAX_STRUCT_DEFINITIONS 32
#define MAX_ENTRIES 256


typedef struct Entry_s {
    // std140 meta-data
    size_t std140_size;
    size_t std140_offset;
    size_t std140_alignment;

    // C size
    size_t type_size; //for arrays and structs, this is the same as the std140 size.

    bool is_struct; // could be an enum instead of two flags
    bool is_sampler;

    int struct_number; //the index in the block's struct definitions array that this entry is an instance of.
    bool is_array; // both is_struct and is_array can be true, for an array of structs.
    int array_length;
    int type; //in symbol table, e.g. "vec4", or a defined struct.
    int the_rest; //in symbol table, e.g. "stuff[NUM_STUFF]".
} Entry;
typedef struct StructDefinition_s {
    // std140 meta-data
    size_t std140_size;
    // no "C_size" as the C struct is padded to the std140 size.

    int name; //in symbol table
    int num_entries;
    Entry entries[MAX_ENTRIES];
} StructDefinition;
typedef struct HashDefinition_s {
    int identifier;
    int value;
} HashDefinition;
typedef struct Block_s {
    int name; //in symbol table
    int num_hash_defines;
    HashDefinition hash_defines[MAX_HASH_DEFINES]; // lookups into the "symbol" table for identifiers, and int values.
    int num_struct_definitions;
    StructDefinition struct_definitions[MAX_STRUCT_DEFINITIONS];
    int num_entries;
    Entry entries[MAX_ENTRIES];
} Block;

// The parser is using g_block to do some semantics, finding past definitions of integer values for array lengths.
extern Block g_block;
extern Block g_struct_definition;

/*
A block has
    - a name.
    - a collection of #defines, in order, as literal strings (no difference in syntax for glsl and C here).
    - a collection of struct definitions, in order, as
        - a name
        - a collection of entries, as
            - a type name, OR a struct identifier.
            - the rest as a literal string (same syntax in glsl and C, so array suffixes are taken into account)
    - a collection of entries, as
        - a type name, OR a struct identifier.
        - the rest as a literal string (same syntax in glsl and C, so array suffixes are taken into account)
*/

size_t std140_layout(Block *block, Entry *entries, int num_entries);
size_t glsl_type_size(char *type);
void new_block(void);
void finish_block(int block_name);
void block_add_entry(Entry entry);
void block_add_struct(int name);
void block_add_hash_define(HashDefinition hash_definition);
Entry new_entry(int type, int the_rest);
void new_struct(void);
void struct_add_entry(Entry entry);

#endif // HEADER_DEFINED_GEN_SHADER_BLOCKS

