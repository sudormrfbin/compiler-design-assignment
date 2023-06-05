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
      ast_free($exp);
      printf("> ");
  }
  | calclist EOL { printf("> "); } /* blank line or comment */
  ;

exp: factor
  | exp '+' factor { $$ = ast_alloc(BinaryAExpr($1, BinaryOp_Add, $3)); }
  | exp '-' factor { $$ = ast_alloc(BinaryAExpr($1, BinaryOp_Sub, $3)); }
  ;

factor: term
  | factor '*' term { $$ = ast_alloc(BinaryAExpr($1, BinaryOp_Mul, $3)); }
  | factor '/' term { $$ = ast_alloc(BinaryAExpr($1, BinaryOp_Div, $3)); }
  ;

term:
  NUMBER        { $$ = ast_alloc(Number($1));               }
  | '-' term    { $$ = ast_alloc(UnaryAExpr(UnaryOp_Minus, $2)); }
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

