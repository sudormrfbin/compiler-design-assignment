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

%type <ast> exp

/* Explicitly declare precedence instead of implicitly doing it
   through the grammar.
*/
%left '-' '+'
%left '*' '/'
%nonassoc UMINUS

%%

calclist: /* nothing */
  | calclist exp EOL {
      printf("= %4.4g\n", eval($exp)); 
      ast_free($exp);
      printf("> ");
  }
  | calclist EOL { printf("> "); } /* blank line or comment */
  ;

exp: exp '+' exp { $$ = ast_alloc(BinaryAExpr($1, BinaryOp_Add, $3)); }
  | exp '-' exp  { $$ = ast_alloc(BinaryAExpr($1, BinaryOp_Sub, $3)); }
  | exp '*' exp  { $$ = ast_alloc(BinaryAExpr($1, BinaryOp_Mul, $3)); }
  | exp '/' exp  { $$ = ast_alloc(BinaryAExpr($1, BinaryOp_Div, $3)); }
  | '-' exp %prec UMINUS { $$ = ast_alloc(UnaryAExpr(UnaryOp_Minus, $2));     }
  | '(' exp ')'  { $$ = $2;                                         }
  | NUMBER       { $$ = ast_alloc(Number($1));                        }
  ;

%%
