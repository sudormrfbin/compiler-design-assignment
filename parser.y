/* Enable detailed error reporting */
/* %define parse.lac full */
%define parse.error detailed

%{
#include <stdio.h>
#include "ast.h"
#include "datatype99.h"

// int yyerror(const char *s) {
//   fprintf(stderr, "error: %s\n", s);
// }

int yylex();

%}

%union {
  ArithExpr *arith_expr;
  BoolExpr *bool_expr;
  Expr *expr;
  Stmt *stmt;
  double number;
}

%token <number> NUMBER

%type <arith_expr> aexpr
%type <bool_expr> bexpr
%type <expr> expr
%type <stmt> stmt display-stmt expr-stmt

%token EOL
%token GT
%token GTE
%token LT
%token LTE
%token AND
%token OR
%token EQEQ

%token TRUE
%token FALSE

%token DISPLAY

/* Explicitly declare precedence instead of implicitly doing it
   through the grammar.
*/
%left EQEQ AND OR
%nonassoc '!'
%left '-' '+'
%left '*' '/'
%nonassoc UMINUS

%%

program: /* nothing */
  | program stmt {
      eval_stmt($stmt);
      ast_free_stmt($stmt);
  }
  | program EOL { } /* blank line or comment */
  ;

stmt: expr-stmt
  | display-stmt;

display-stmt: DISPLAY expr EOL {{ $$ = stmt_alloc(DisplayStmt($expr)); }};

expr-stmt: expr EOL {{ $$ = stmt_alloc(ExprStmt($expr)); }};

expr: aexpr { $$ = expr_alloc(ArithmeticExpr($1)); }
  | bexpr { $$ = expr_alloc(BooleanExpr($1)); }
  ;

/* Arithmetic expression */
aexpr: aexpr '+' aexpr { $$ = aexpr_alloc(BinaryAExpr($1, BinaryOp_Add, $3)); }
  | aexpr '-' aexpr    { $$ = aexpr_alloc(BinaryAExpr($1, BinaryOp_Sub, $3)); }
  | aexpr '*' aexpr    { $$ = aexpr_alloc(BinaryAExpr($1, BinaryOp_Mul, $3)); }
  | aexpr '/' aexpr    { $$ = aexpr_alloc(BinaryAExpr($1, BinaryOp_Div, $3)); }
  | '-' aexpr %prec UMINUS { $$ = aexpr_alloc(UnaryAExpr(UnaryOp_Minus, $2)); }
  | '(' aexpr ')'      { $$ = $2;                       }
  | NUMBER             { $$ = aexpr_alloc(Number($1));  }
  ;

/* Boolean exression */
bexpr:
    aexpr EQEQ aexpr { $$ = bexpr_alloc(RelationalArithExpr($1, RelationalEqual, $3)); }
  | aexpr GT   aexpr { $$ = bexpr_alloc(RelationalArithExpr($1, Greater, $3));         }
  | aexpr GTE  aexpr { $$ = bexpr_alloc(RelationalArithExpr($1, GreaterOrEqual, $3));  }
  | aexpr LT   aexpr { $$ = bexpr_alloc(RelationalArithExpr($1, Less, $3));            }
  | aexpr LTE  aexpr { $$ = bexpr_alloc(RelationalArithExpr($1, LessOrEqual, $3));     }
  | bexpr AND  bexpr { $$ = bexpr_alloc(LogicalBoolExpr($1, And, $3));                 }
  | bexpr OR   bexpr { $$ = bexpr_alloc(LogicalBoolExpr($1, Or, $3));                  }
  | bexpr EQEQ bexpr { $$ = bexpr_alloc(LogicalBoolExpr($1, LogicalEqual, $3));        }
  | '!' bexpr { $$ = bexpr_alloc(NegatedBoolExpr($2)); }
  | TRUE      { $$ = bexpr_alloc(Boolean(true));       }
  | FALSE     { $$ = bexpr_alloc(Boolean(false));      }

%%
