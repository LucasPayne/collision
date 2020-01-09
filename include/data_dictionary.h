/*================================================================================
Data dictionaries
-----------------
This is a system for describing structured, typed, human-readable and editable data,
distributed over multiple files, with data inheritence.
Possible uses are:
    - Application configuration data.
    - Type schemas and object prefabrications, for instancing.
    - Collections data for serialized scenes.

Data is distributed through .dd files which can include eachother literally.

The central concepts in this data-description language are
    - Dictionaries.
    - Typed values.
    - Dictionary-expressions.

A .dd file, when recursively flattened by inclusion of other .dd files, is a heirarchical
text database. Each dictionary can be named, and contain other dictionaries, named or unnamed,
and typed key-value pairs.

A dictionary-expression is a sequence of dictionaries. The expression is resolved by successively "masking"
dictionaries, which (at the dictionary level, _not recurring_):
    - Adds typed key-value pairs with new names.
    - Fails if a key-value pair is overwritten with a key-value pair with the same name, but different type.
    - Adds new unnamed dictionaries or dictionaries with new names.
    - Appends the dictionary-expression of a dictionary with the same name to the current expression.

The appending of dictionary-expressions achieves a "defered recursion": the resolution of the contents of a dictionary
is logically "masking a tree", but these further dictionary masks are only resolved when that sub-dictionary is queried.

Data in a .dd file is described with a simple syntax. When defining a new dictionary,
the name is on the left, followed by "<" (think of this as "injecting" the following contents into the dictionary),
and then a dictionary-expression, a sequence of dictionaries. A dictionary-expression can contain literal dictionaries, defined in parentheses,
and/or named dictionaries, just the name as a symbol.
Key value pairs have an _optional_ type. The type need not be included if the dictionary the entry is in is intended to masked onto another
where the type is declared. Both dictionary and entry definitions end in a semicolon.
This simple syntax along with the dictionary-expression mechanism gives all the data-describing ability of .dd files.

Data inheritence, type-heirarchies, default values, composite objects, collections data, are all possible using this syntax.

For example, here is an example of "objects" and a dictionary of instances of these "objects".
data.dd
---
Body < (
    int size: 1;
    bool is_square: true;
);
Thing < (
    int a: 5;
    vec4 v: 1,2,3,4;
    body < Body (
        size: 100;
    );
);
Scene < (#include(stuff.dd););

stuff.dd
---
< Thing (
    a: 2;
);
< Thing (
    body < (
        is_square: false;
    );
);
< Thing (
    v: 0,0,0,0;
    body < (
        size: 0.01;
    );
);
================================================================================*/
#ifndef HEADER_DEFINED_DATA_DICTIONARY
#define HEADER_DEFINED_DATA_DICTIONARY
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>


//--------------------------------------------------------------------------------
// ---Really have to learn how to create an interface. Should these structs be here?
// AST
struct DictExpression_s;
struct EntryNode_s;
typedef struct DictExpression_s {
    struct DictExpression_s *next;
    bool is_name;
    // Kind: name
    int name; //symbol
    // Kind: dict-literal
    struct EntryNode_s *dict;
} DictExpression;
typedef struct EntryNode_s {
    struct EntryNode_s *next;
    bool is_dict;
    int name; //symbol
    // Kind: entry
    int type; //symbol, -1 as no type.
    int value_text; // symbol, -1 as no value text.
    // Kind: dict
    struct DictExpression_s *dict_expression;
} EntryNode;
// Dictionary tables (hash table where dictionaries are masked together)
typedef struct DictionaryTableCell_s {
    int name; // -1: empty cell.
    union {
        struct {
            int32_t type;
            int32_t value_text;
        } value;
        struct {
            DictExpression *dict_expression;
            int num_types;
            int *types; //dynamic array, consisting of all names of named dictionaries in expanded dict-expression.
        } dict;
    } contents;
    bool is_dict;
} DictionaryTableCell;
typedef struct DataDictionary_s {
    // Be careful when allocating memory for this "variable-size struct".
    int table_size;
    struct DataDictionary_s *parent_dictionary; // for scoping.
    DictionaryTableCell *table;
    DictionaryTableCell ___table;
} DataDictionary;

// Open a dictionary from an actual path, e.g. a .dd file.
DataDictionary *dd_fopen(char *path);

//--- Possibly shouldn't have this here. But it is useful for debugging.
//The actual printing should probably not show the hash-table data structure.
void dd_print_table(DataDictionary *dict_table);

// The proper printing utility for users.
void dd_print(DataDictionary *dd);

// Open a subdictionary.
DataDictionary *dd_open(DataDictionary *dict, char *name);
// Query for a value.
bool dd_get(DataDictionary *dict, char *name, char *type, void *data);


// Type readers.
typedef bool (*DDTypeReader)(const char *, void *);
DDTypeReader dd_get_reader(const char *type);

#endif // HEADER_DEFINED_DATA_DICTIONARY
