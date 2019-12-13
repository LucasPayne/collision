/*
flex and bison p28
*/
%option noyywrap nodefault
%x Preprocessing
%x AngleNameGet
%x QuotesNameGet

%{

struct BufferStackEntry {
    struct BufferStackEntry *prev;
    YY_BUFFER_STATE buffer_state;
    char *filename;
    FILE *file;
} CurBS;

static FILE *system_file_open(char *filename);
static FILE *local_file_open(char *filename);
static void preprocess(FILE *file);
%}

%%

<Preprocessing>^"#include <" {
    printf("Beginning system include ...\n");
    BEGIN AngleNameGet;
}
<Preprocessing>^"#include \"" {
    printf("Beginning local include ...\n");
    BEGIN QuotesNameGet;
}
<Preprocessing>.|"\n" {
    putchar(yytext[0]);
}
<AngleNameGet>[^\n]*>$ {
    char name[1024];
    strncpy(name, yytext, strlen(yytext) - 1);
    printf("Including file %s ...\n", name);
    FILE *file = system_file_open(name);
    preprocess(file);
    BEGIN Preprocessing;
}
<QuotesNameGet>[^\n]*\"$ {
    char name[1024];
    strncpy(name, yytext, strlen(yytext) - 1);
    printf("Including file %s ...\n", name);
    FILE *file = local_file_open(name);
    preprocess(file);
    BEGIN Preprocessing;
}
<AngleNameGet,QuotesNameGet>.|"\n" {
    fprintf(stderr, "ERROR: Invalid preprocessor #include directive.\n");
    exit(EXIT_FAILURE);
}


%%

static FILE *system_file_open(char *filename)
{
    return fopen(filename, "r");
}
static FILE *local_file_open(char *filename)
{
    return fopen(filename, "r");
}

static void preprocess(FILE *file)
{
    if (file == NULL) {
        fprintf(stderr, "ERROR: Attempted to preprocess a NULL file.\n");
        exit(EXIT_FAILURE);
    }
    printf("Preprocessing file ...\n");
    yyin = file;
    BEGIN Preprocessing;
    yylex();
}


int main(int argc, char *argv[])
{
    if (argc != 2) { printf("give good args\n"); exit(1); }
    char *filename = argv[1];

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "ERROR: Could not open file \"%s\".\n", filename);
        exit(EXIT_FAILURE);
    }
    preprocess(file);
}
