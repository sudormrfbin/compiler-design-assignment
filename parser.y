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
%type <statement_list> stmt-list
%type <else_if> else-if-chain

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

%token IF
%token THEN
%token ELSE
%token ENDIF

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
    stmt_list_add(&ptr, $stmt);
    $$ = ptr;
  }
  | stmt-list stmt {
    stmt_list_add(&$1, $2);
    $$ = $1;
  }
  ;

stmt: expr-stmt
  | display-stmt
  | if-stmt
  | assign-stmt
  ;

assign-stmt: IDENT '=' expr eol {
    $$ = stmt_alloc(AssignStmt($1, $expr));
  }
  ;

/* TODO: extract out else-stmts */
if-stmt: IF bexpr THEN eol stmt-list else-if-chain ENDIF eol {
    $$ = stmt_alloc(IfStmt($bexpr, $[stmt-list], $[else-if-chain], NULL));
  }
  | IF bexpr THEN eol stmt-list[true-stmts] else-if-chain ELSE eol stmt-list[else-stmts] ENDIF eol {
    $$ = stmt_alloc(IfStmt($bexpr, $[true-stmts], $[else-if-chain], $[else-stmts]));
  }
  ;

else-if-chain: { $$ = NULL; }
  | else-if-chain ELSE IF bexpr THEN eol stmt-list {
    else_if_add(&$1, $bexpr, $[stmt-list]);
    $$ = $1;
  }
  ;

display-stmt: DISPLAY expr eol { $$ = stmt_alloc(DisplayStmt($expr)); };

expr-stmt: expr eol { $$ = stmt_alloc(ExprStmt($expr)); };

expr: aexpr { $$ = expr_alloc(ArithmeticExpr($1)); }
  | bexpr { $$ = expr_alloc(BooleanExpr($1)); }
  | sexpr { $$ = expr_alloc(StringExpr($1)); }
  | iexpr { $$ = expr_alloc(IdentExpr($1)); }
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

sexpr: STRING { $$ = sexpr_alloc(String($1)); }
  | sexpr '+' sexpr { $$ = sexpr_alloc(StringConcat($1, $3)); }
  /* TODO: Add == */
  ;

iexpr: IDENT
  ;

%%
