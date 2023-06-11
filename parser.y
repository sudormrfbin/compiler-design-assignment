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
  LiteralExpr *literal_expr;
  IdentBinaryOp ident_bop;
  IdentUnaryOp ident_uop;
  IdentExpr *ident_expr;
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
%type <literal_expr> literal-expr
%type <ident_expr> ident-expr
%type <ident_bop> ident-binary-op
%type <ident_uop> ident-unary-op
%type <expr> expr
%type <stmt> stmt display-stmt expr-stmt if-stmt assign-stmt while-stmt for-stmt
%type <statement_list> stmt-list then-clause else-clause
%type <else_if> else-if-chain

%token EOL
%token GT GTE LT LTE
%token TRUE FALSE
%token DISPLAY
%token IF THEN ELSE ENDIF
%token DO
%token WHILE ENDWHILE
%token FOR TO ENDFOR

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

eol: EOL
  | eol EOL

stmt-list: stmt {
    StatementList* ptr = NULL;
    add_stmt_list(&ptr, $stmt);
    $$ = ptr;
  }
  | stmt-list stmt {
    add_stmt_list(&$1, $2);
    $$ = $1;
  }

stmt: expr-stmt
  | display-stmt
  | if-stmt
  | while-stmt
  | for-stmt
  | assign-stmt

assign-stmt: IDENT '=' expr eol { $$ = alloc_stmt(AssignStmt($1, $expr)); }

display-stmt: DISPLAY expr eol { $$ = alloc_stmt(DisplayStmt($expr)); }

if-stmt: IF expr then-clause else-if-chain else-clause ENDIF eol {
    $$ = alloc_stmt(IfStmt($expr, $[then-clause], $[else-if-chain], $[else-clause]));
  }

then-clause: THEN eol stmt-list { $$ = $[stmt-list]; }

else-if-chain: %empty { $$ = NULL; }
  | else-if-chain ELSE IF expr then-clause {
    add_else_if(&$1, $expr, $[then-clause]);
    $$ = $1;
  }

else-clause: %empty { $$ = NULL; }
  | ELSE eol stmt-list { $$ = $[stmt-list]; }

while-stmt: WHILE expr DO eol stmt-list ENDWHILE eol {
  $$ = alloc_stmt(WhileStmt($expr, $[stmt-list]));
}

for-stmt: FOR IDENT '=' expr[start] TO expr[end] DO eol stmt-list ENDFOR eol {
  $$ = alloc_stmt(ForStmt($2, $start, $end, $[stmt-list]));
}

expr-stmt: expr eol { $$ = alloc_stmt(ExprStmt($expr)); }

ident-binary-op: '+' { $$ = IdentBOp_Plus; }
  | '-'  { $$ = IdentBOp_Minus; }
  | '*'  { $$ = IdentBOp_Star;  }
  | '/'  { $$ = IdentBOp_Slash; }
  | GT   { $$ = IdentBOp_Gt;    }
  | GTE  { $$ = IdentBOp_Gte;   }
  | LT   { $$ = IdentBOp_Lt;    }
  | LTE  { $$ = IdentBOp_Lte;   }
  | EQEQ { $$ = IdentBOp_EqEq;  }
  | AND  { $$ = IdentBOp_And;   }
  | OR   { $$ = IdentBOp_Or;    }

ident-unary-op: '!' { $$ = IdentUOp_Exclamation; }
  | '-' { $$ = IdentUOp_Minus; }

expr: literal-expr { $$ = alloc_expr(LiteralExpression($1)); }
  | ident-expr { $$ = alloc_expr(IdentExpression($1)); }

ident-expr:
  IDENT ident-binary-op literal-expr { $$ = alloc_ident_expr(IdentBinaryExpr($1, $2, $3)); }
  | ident-unary-op IDENT { $$ = alloc_ident_expr(IdentUnaryExpr($1, $2)); }
  | IDENT { $$ = alloc_ident_expr(Identifier($1)); }

literal-expr: aexpr { $$ = alloc_literal_expr(ArithmeticExpr($1)); }
  | bexpr { $$ = alloc_literal_expr(BooleanExpr($1)); }
  | sexpr { $$ = alloc_literal_expr(StringExpr($1)); }

/* Arithmetic expression */
aexpr: aexpr '+' aexpr { $$ = alloc_aexpr(BinaryAExpr($1, BinaryOp_Add, $3)); }
  | aexpr '-' aexpr    { $$ = alloc_aexpr(BinaryAExpr($1, BinaryOp_Sub, $3)); }
  | aexpr '*' aexpr    { $$ = alloc_aexpr(BinaryAExpr($1, BinaryOp_Mul, $3)); }
  | aexpr '/' aexpr    { $$ = alloc_aexpr(BinaryAExpr($1, BinaryOp_Div, $3)); }
  | '-' aexpr %prec UMINUS { $$ = alloc_aexpr(UnaryAExpr(UnaryOp_Minus, $2)); }
  | '(' aexpr ')'      { $$ = $2;                       }
  | NUMBER             { $$ = alloc_aexpr(Number($1));  }

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
