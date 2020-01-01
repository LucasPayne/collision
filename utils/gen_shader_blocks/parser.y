
%{
#include "gen_shader_blocks.h"
#include <string.h>
%}
%union {
    // storing the token information for a "#define MAX_NUMBER_LIGHTS 32" as the MAX_NUMBER_LIGHTS token and the value 32.
    HashDefinition hash_definition;
    int symbol;
    Entry entry;
}
%token BLOCK
%token STRUCT
%token LEFTBRACE
%token RIGHTBRACE
%token LEFTBRACKET
%token RIGHTBRACKET
%token SEMICOLON
%token <symbol> IDENTIFIER
%token <hash_definition> HASHDEFINE

%type <entry> entry

%%

file: /* nothing */ | file block
    ;
block: {
    // pre-expansion code with an embedded action (flex & bison p143 (163))
    printf("Creating a new block.\n");
    new_block();
} BLOCK IDENTIFIER LEFTBRACE block_entries RIGHTBRACE SEMICOLON {
    printf("block created: %s\n", symbol($3));
    finish_block($3);
    return 0; // success, return for the processing one block.
};
block_entries: /* epsilon */ | block_entry block_entries
block_entry:
    entry {
        block_add_entry($1);
    }
    | struct
    | HASHDEFINE {
        printf("block hash_define: %s, value: %d\n", symbol($1.identifier), $1.value);
        block_add_hash_define($1);
    };

entry: IDENTIFIER IDENTIFIER SEMICOLON {
    // Singleton entry.
    printf("entry: type: %s, name: %s\n", symbol($1), symbol($2));
    Entry entry = new_entry($1, $2);
    entry.is_array = false;
    $$ = entry;
    printf("type_size: %zu\n", $$.type_size);
}
| IDENTIFIER IDENTIFIER LEFTBRACKET IDENTIFIER RIGHTBRACKET SEMICOLON {
    // Array entry with non-literal (macro e.g. NUM_LIGHTS) length.
    char *length_token = symbol($4);
    Entry entry = new_entry($1, $2);
    entry.is_array = true;
    int i;
    int length;
    for (i = 0; i < g_block.num_hash_defines; i++) {
        if (strcmp(length_token, symbol(g_block.hash_defines[i].identifier)) == 0) {
            // #define NUM_LIGHTS 32;
            // ...
            // Light lights[NUM_LIGHTS]; -> array entry, type: Light, name: lights, length: 32.
            length = g_block.hash_defines[i].value;
            break;
        }
    }
    if (i == g_block.num_hash_defines) {
        fprintf(stderr, "ERROR: Array length specified as token \"%s\", which has not been defined above this occurence in the block.\n", length_token);
        exit(EXIT_FAILURE);
    }
    entry.array_length = length;
    $$ = entry;
    printf("type_size: %zu\n", $$.type_size);
}
struct: {
    printf("Creating a new struct.\n");
    new_struct();
} STRUCT IDENTIFIER LEFTBRACE struct_entries RIGHTBRACE SEMICOLON {
    printf("struct definition: %s\n", symbol($3));
    block_add_struct($3);
};
struct_entries: /* epsilon */ | entry struct_entries {
    struct_add_entry($1);
};

%%
/*
typedef struct Entry_s {
    int type; //in symbol table, e.g. "vec4", or a defined struct.
    int the_rest; //in symbol table, e.g. "stuff[NUM_STUFF]".
} Entry;
typedef struct StructDefinition_s {
    int name; //in symbol table
    Entry entries[MAX_ENTRIES];
} StructDefinition;
typedef struct Block_s {
    int name; //in symbol table
    int hash_defines[MAX_HASH_DEFINES]; // lookups into the "symbol" table
    StructDefinition *struct_definition[MAX_STRUCT_DEFINITIONS];
    Entry entries[MAX_ENTRIES];
} Block;


FILE ::= BLOCK*
BLOCK ::= block IDENTIFIER { (ENTRY | STRUCT | DEFINITION)* };
IDENTIFIER ::= //a valid glsl identifier.
ENTRY ::= TYPE IDENTIFIER;
TYPE ::= //a valid glsl type or [struct that has been defined above in the file].
STRUCT ::= struct IDENTIFIER IDENTIFIER { ENTRY* }; //first identifier is for the block to attach to (does not need to be already defined), second is for the struct name.
DEFINITION ::= //a valid C #define definition.

    | STRUCT IDENTIFIER IDENTIFIER SEMICOLON {
        printf("struct entry: type: %s, name: %s\n", symbol($2), symbol($3));
    };
*/
