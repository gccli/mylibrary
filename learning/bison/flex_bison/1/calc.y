/* simplest version of calculator */
%{
    #include <stdio.h>
%}
/* declare tokens */
%token NUMBER
%token ADD SUB MUL DIV ABS
%token OP CP
%token EOL

%%
calclist: /* nothing */ /*matches at beginning of input*/
| calclist exp EOL { printf("= %d\n", $2); } //EOL is end of an expression
;
exp: factor// default $$ = $1
     | exp ADD factor { $$ = $1 + $3; }
     | exp SUB factor { $$ = $1 - $3; }
     ;
factor: term //default $$ = $1
     | factor MUL term { $$ = $1 * $3; }
     | factor DIV term { $$ = $1 / $3; }
     ;
term: NUMBER //default $$ = $1
     | ABS term { $$ = $2 >= 0? $2 : - $2; }
     | OP exp CP { $$ = $2; }
     ;
%%
int main(int argc, char **argv)
{
    yyparse();
}
void yyerror(char *s)
{
    fprintf(stderr, "error: %s\n", s);
}
