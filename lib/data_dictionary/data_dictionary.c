#include <stdio.h>
#include <stdlib.h>
#include "data_dictionary.h"
#include "data_dictionary_implementation.h"

DataDictionary *dd_open(char *path)
{
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        printf("Could not find data-dictionary file \"%s\".\n", path);
        return NULL;
    }
    dd_push_file(file);
    dd_yyparse();
    
    // Formulate the top-level dictionary as a 1-operand dict-expression, then open it.
    DictExpression top_expression = { 0 };
    top_expression.is_name = false;
    top_expression.dict = g_dict; // g_dict has been left by the parser as the top-level dictionary IR.
    DataDictionary *dict = resolve_dictionary_expression(NULL, &top_expression);
    if (dict == NULL) {
        printf("Something went wrong when trying to open data-dictionary file \"%s\".\n", path);
        return NULL;
    }
    return dict;
}
