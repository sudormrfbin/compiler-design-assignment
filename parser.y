/* Enable detailed error reporting */
%define parse.lac full
%define parse.error detailed

%{

#include <stdio.h>
#include "ast.h"
#include "datatype99.h"

/* Global variable for storing the resulting AST after parsing a file */
StatementList* parse_result = NULL;

/* Global variable pointing to the symbol table */
SymbolTable* symtab = NULL;

int yylex();

%}

%union {
  StrExpr *str_expr;
  ArithExpr *arith_expr;
  BoolExpr *bool_expr;
  Expr *expr;
  Stmt *stmt;
  StatementList *statement_list;
  ElseIfStatement *else_if;
  double number;
  char* string;
  char* ident;
}

%token <number> NUMBER
%token <string> STRING
%token <ident>  IDENT

%type <str_expr> sexpr
%type <arith_expr> aexpr
%type <bool_expr> bexpr
%type <ident> iexpr
%type <expr> expr
%type <stmt> stmt display-stmt expr-stmt if-stmt assign-stmt
%type <statement_list> stmt-list then-clause else-clause
%type <else_if> else-if-chain

%token EOL
%token GT GTE LT LTE EQEQ
%token AND OR
%token TRUE FALSE
%token DISPLAY
%token IF THEN ELSE ENDIF

/* Explicitly declare precedence instead of implicitly doing it
   through the grammar.
*/
%left EQEQ AND OR
%nonassoc '!'
%left '-' '+'
%left '*' '/'
%nonassoc UMINUS

%%

program: stmt-list { parse_result = $1; }
  | eol stmt-list  { parse_result = $2; }
  ;

eol: EOL
  | eol EOL
  ;

stmt-list: stmt {
    StatementList* ptr = NULL;
    add_stmt_list(&ptr, $stmt);
    $$ = ptr;
  }
  | stmt-list stmt {
    add_stmt_list(&$1, $2);
    $$ = $1;
  }
  ;

stmt: expr-stmt
  | display-stmt
  | if-stmt
  | assign-stmt
  ;

assign-stmt: IDENT '=' expr eol {
    $$ = alloc_stmt(AssignStmt($1, $expr));
  }
  ;

display-stmt: DISPLAY expr eol { $$ = alloc_stmt(DisplayStmt($expr)); }

if-stmt: IF bexpr then-clause else-if-chain else-clause ENDIF eol {
    $$ = alloc_stmt(IfStmt($bexpr, $[then-clause], $[else-if-chain], $[else-clause]));
  }

then-clause: THEN eol stmt-list { $$ = $[stmt-list]; }

else-if-chain: { $$ = NULL; }
  | else-if-chain ELSE IF bexpr THEN eol stmt-list {
    add_else_if(&$1, $bexpr, $[stmt-list]);
    $$ = $1;
  }
  ;

else-clause: ELSE eol stmt-list { $$ = $[stmt-list]; }

expr-stmt: expr eol { $$ = alloc_stmt(ExprStmt($expr)); }

expr: aexpr { $$ = alloc_expr(ArithmeticExpr($1)); }
  | bexpr { $$ = alloc_expr(BooleanExpr($1)); }
  | sexpr { $$ = alloc_expr(StringExpr($1)); }
  | iexpr { $$ = alloc_expr(IdentExpr($1)); }
  ;

/* Arithmetic expression */
aexpr: aexpr '+' aexpr { $$ = alloc_aexpr(BinaryAExpr($1, BinaryOp_Add, $3)); }
  | aexpr '-' aexpr    { $$ = alloc_aexpr(BinaryAExpr($1, BinaryOp_Sub, $3)); }
  | aexpr '*' aexpr    { $$ = alloc_aexpr(BinaryAExpr($1, BinaryOp_Mul, $3)); }
  | aexpr '/' aexpr    { $$ = alloc_aexpr(BinaryAExpr($1, BinaryOp_Div, $3)); }
  | '-' aexpr %prec UMINUS { $$ = alloc_aexpr(UnaryAExpr(UnaryOp_Minus, $2)); }
  | '(' aexpr ')'      { $$ = $2;                       }
  | NUMBER             { $$ = alloc_aexpr(Number($1));  }
  ;

/* Boolean exression */
bexpr:
    aexpr EQEQ aexpr { $$ = alloc_bexpr(RelationalArithExpr($1, RelationalEqual, $3)); }
  | aexpr GT   aexpr { $$ = alloc_bexpr(RelationalArithExpr($1, Greater, $3));         }
  | aexpr GTE  aexpr { $$ = alloc_bexpr(RelationalArithExpr($1, GreaterOrEqual, $3));  }
  | aexpr LT   aexpr { $$ = alloc_bexpr(RelationalArithExpr($1, Less, $3));            }
  | aexpr LTE  aexpr { $$ = alloc_bexpr(RelationalArithExpr($1, LessOrEqual, $3));     }
  | bexpr AND  bexpr { $$ = alloc_bexpr(LogicalBoolExpr($1, And, $3));                 }
  | bexpr OR   bexpr { $$ = alloc_bexpr(LogicalBoolExpr($1, Or, $3));                  }
  | bexpr EQEQ bexpr { $$ = alloc_bexpr(LogicalBoolExpr($1, LogicalEqual, $3));        }
  | '!' bexpr { $$ = alloc_bexpr(NegatedBoolExpr($2)); }
  | TRUE      { $$ = alloc_bexpr(Boolean(true));       }
  | FALSE     { $$ = alloc_bexpr(Boolean(false));      }

sexpr: STRING { $$ = alloc_sexpr(String($1)); }
  | sexpr '+' sexpr { $$ = alloc_sexpr(StringConcat($1, $3)); }
  /* TODO: Add == */
  ;

iexpr: IDENT

%%
