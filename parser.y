/* Enable detailed error reporting */
/* %define parse.lac full */
%define parse.error detailed

%{
#include <stdio.h>
#include "ast.h"

// int yyerror(const char *s) {
//   fprintf(stderr, "error: %s\n", s);
// }

int yylex();

%}

%union {
  Ast *ast;
  double number;
}

%token <number> NUMBER
%token EOL

%type <ast> exp factor term

%%

calclist: /* nothing */
  | calclist exp EOL {
      printf("= %4.4g\n", eval($exp)); 
      treefree($exp);
      printf("> ");
  }
  | calclist EOL { printf("> "); } /* blank line or comment */
  ;

exp: factor
  | exp '+' factor { $$ = newast('+', $1, $3); }
  | exp '-' factor { $$ = newast('-', $1, $3); }
  ;

factor: term
  | factor '*' term { $$ = newast('*', $1, $3); }
  | factor '/' term { $$ = newast('/', $1, $3); }
  ;

term:
  NUMBER        { $$ = newnum($1);               }
  | '|' term    { $$ = newast('|', $2, NULL); }
  | '-' term    { $$ = newast('M', $2, NULL); }
  | '(' exp ')' { $$ = $exp;                     }
  ;

%%

// int main(int argc, char **argv) {
  // if a file is given as command line argument, set input
  // to file contents.
  // if (argc > 1) {
  //   if (!(yyin = fopen(argv[1], "r"))) {
  //     perror(argv[1]);
  //     return 1;
  //   }
  // }

  // yyparse();
// }

