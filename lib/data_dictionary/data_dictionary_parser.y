
%{
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "data_dictionary_implementation.h"

EntryNode *g_dict;

#define tracing_parse 1

%}
%union {
    int symbol;
    EntryNode *entry_node;
    DictExpression *dict_expression;
}
%token <symbol> IDENTIFIER
%token <symbol> VALUE_TEXT

%type <entry_node> Dict // Dict is head of the linked list.
%type <entry_node> Pair
%type <dict_expression> DictExpression

%%

Dict:
    Pair Dict {
        $1->next = $2;
        // This pair is now the head of the linked list.
        $$ = $1;
        g_dict = $$; // is there a better way to do this? After yyparse(), this will be the top-level dictionary, so it is a way to return the AST.
        if (tracing_parse) printf("Dict: Pair Dict\n");
    }
    | /* epsilon */ {
        $$ = NULL;
        if (tracing_parse) printf("Dict:\n");
    }

    /* "pair": optionally typed key-value pairs and named and nameless < dictionary injections with dict-expression. */
Pair:
    IDENTIFIER IDENTIFIER ';' {
        $$ = new_entry_node($2, $1, -1);
        if (tracing_parse) printf("Pair: IDENTIFIER=\"%s\" IDENTIFIER=\"%s\";\n", symbol($1), symbol($2));
    }
    | IDENTIFIER IDENTIFIER ':' VALUE_TEXT ';' {
        $$ = new_entry_node($2, $1, $4);
        if (tracing_parse) printf("Pair: IDENTIFIER=\"%s\" IDENTIFIER=\"%s\": VALUE_TEXT=\"%s\";\n", symbol($1), symbol($2), symbol($4));
    }
    | IDENTIFIER ':' VALUE_TEXT ';' {
        $$ = new_entry_node($1, -1, $3);
        if (tracing_parse) printf("Pair: IDENTIFIER=\"%s\": VALUE_TEXT=\"%s\";\n", symbol($1), symbol($3));
    }
    | IDENTIFIER '<' DictExpression ';' {
        $$ = new_dict_node($1, $3);
        if (tracing_parse) printf("Pair: IDENTIFIER=\"%s\" < DictExpression;\n", symbol($1));
    }
    | '<' DictExpression ';' {
        $$ = new_dict_node(-1, $2);
        if (tracing_parse) printf("Pair: < DictExpression;\n");
    }

DictExpression:
    '(' Dict ')' DictExpression {
        $$ = new_literal_dict_expression($2);
        $$->next = $4;
        if (tracing_parse) printf("DictExpression: (Dict) DictExpression\n");
    }
    | IDENTIFIER DictExpression {
        $$ = new_named_dict_expression($1);
        $$->next = $2;
        if (tracing_parse) printf("DictExpression: IDENTIFIER=\"%s\" DictExpression\n", symbol($1));
    }
    | /* epsilon */ {
        $$ = NULL;
        if (tracing_parse) printf("DictExpression:\n");
    }

%%

