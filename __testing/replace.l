%{
%}

DIGIT [0-9]

%%

username   printf("cool bro"); 
{DIGIT}+   printf("DIGIT FOUND!!!\n");
.          puts(yytext); puts(yytext); 

%%

int main(int argc, char **argv)
{
    yylex();
}
