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
  ArithExpr *ast;
  double number;
}

%token <number> NUMBER
%token EOL

%type <ast> aexpr

/* Explicitly declare precedence instead of implicitly doing it
   through the grammar.
*/
%left '-' '+'
%left '*' '/'
%nonassoc UMINUS

%%

calclist: /* nothing */
  | calclist aexpr EOL {
      printf("= %4.4g\n", eval($aexpr)); 
      ast_free($aexpr);
      printf("> ");
  }
  | calclist EOL { printf("> "); } /* blank line or comment */
  ;

/* Arithmetic expression */
aexpr: aexpr '+' aexpr { $$ = ast_alloc(BinaryAExpr($1, BinaryOp_Add, $3)); }
  | aexpr '-' aexpr    { $$ = ast_alloc(BinaryAExpr($1, BinaryOp_Sub, $3)); }
  | aexpr '*' aexpr    { $$ = ast_alloc(BinaryAExpr($1, BinaryOp_Mul, $3)); }
  | aexpr '/' aexpr    { $$ = ast_alloc(BinaryAExpr($1, BinaryOp_Div, $3)); }
  | '-' aexpr %prec UMINUS { $$ = ast_alloc(UnaryAExpr(UnaryOp_Minus, $2)); }
  | '(' aexpr ')'      { $$ = $2;                                           }
  | NUMBER             { $$ = ast_alloc(Number($1));                        }
  ;

%%
