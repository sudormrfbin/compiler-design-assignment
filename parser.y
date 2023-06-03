/* Enable detailed error reporting */
/* %define parse.lac full */
%define parse.error detailed

%{
#include <stdio.h>

int yyerror(const char *s) {
  fprintf(stderr, "error: %s\n", s);
}

int yylex();

%}

%token NUMBER
%token ADD SUB MUL DIV ABS
%token EOL

%%

calclist: /* nothing */
  | calclist exp EOL { printf("= %d\n", $2); }
  ;

exp: factor
  | exp ADD factor { $$ = $1 + $3; }
  | exp SUB factor { $$ = $1 - $3; }
  ;

factor: term
  | factor MUL term { $$ = $1 * $3; }
  | factor DIV term { $$ = $1 / $3; }
  ;

term: NUMBER
  | ABS term { $$ = $2 >= 0 ? $2 : - $2; }
  ;

%%

int main(int argc, char **argv) {
  // if a file is given as command line argument, set input
  // to file contents.
  // if (argc > 1) {
  //   if (!(yyin = fopen(argv[1], "r"))) {
  //     perror(argv[1]);
  //     return 1;
  //   }
  // }

  yyparse();
}

