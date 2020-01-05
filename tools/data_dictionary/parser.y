%{
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "data_dictionary.h"
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
        printf("Dict: Pair Dict\n");
    }
    | /* epsilon */ {
        $$ = NULL;
        printf("Dict:\n");
    }

    /* "pair": optionally typed key-value pairs and named and nameless < dictionary injections with dict-expression. */
Pair:
    IDENTIFIER IDENTIFIER ';' {
        $$ = new_entry_node($2, $1, -1);
        printf("Pair: IDENTIFIER=\"%s\" IDENTIFIER=\"%s\";\n", symbol($1), symbol($2));
    }
    | IDENTIFIER IDENTIFIER ':' VALUE_TEXT ';' {
        $$ = new_entry_node($2, $1, $4);
        printf("Pair: IDENTIFIER=\"%s\" IDENTIFIER=\"%s\": VALUE_TEXT=\"%s\";\n", symbol($1), symbol($2), symbol($4));
    }
    | IDENTIFIER ':' VALUE_TEXT ';' {
        $$ = new_entry_node($1, -1, $3);
        printf("Pair: IDENTIFIER=\"%s\": VALUE_TEXT=\"%s\";\n", symbol($1), symbol($3));
    }
    | IDENTIFIER '<' DictExpression ';' {
        $$ = new_dict_node($1, $3);
        printf("Pair: IDENTIFIER=\"%s\" < DictExpression;\n", symbol($1));
    }
    | '<' DictExpression ';' {
        $$ = new_dict_node(-1, $2);
        printf("Pair: < DictExpression;\n");
    }

DictExpression:
    '(' Dict ')' DictExpression {
        printf("DictExpression: (Dict) DictExpression\n");
    }
    | IDENTIFIER DictExpression {
        printf("DictExpression: IDENTIFIER=\"%s\" DictExpression\n", symbol($1));
    }
    | /* epsilon */ {
        printf("DictExpression:\n");
    }

%%

