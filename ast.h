#include "datatype99.h"
#include <stdbool.h>

extern int yylineno;
void yyerror(const char *, ...);

typedef enum {
  BinaryOp_Add,
  BinaryOp_Sub,
  BinaryOp_Mul,
  BinaryOp_Div,
} BinaryOp;

typedef enum {
  UnaryOp_Minus,
} UnaryOp;

typedef enum {
  RelationalEqual,
  Greater,
  GreaterOrEqual,
  Less,
  LessOrEqual,
} RelationalOp;

typedef enum {
  And,
  Or,
  LogicalEqual,
} LogicalOp;

datatype(
  ArithExpr,
  (BinaryAExpr, ArithExpr *, BinaryOp, ArithExpr *),
  (UnaryAExpr, UnaryOp, ArithExpr *), // Use NegatedArithExpr and remove UnaryOp
  (Number, double)
);

datatype(
  BoolExpr,
  (RelationalArithExpr, ArithExpr*, RelationalOp, ArithExpr*),
  (LogicalBoolExpr, BoolExpr*, LogicalOp, BoolExpr*),
  (NegatedBoolExpr, BoolExpr*),
  (Boolean, bool)
);

datatype(
  Expr,
  (BooleanExpr, BoolExpr *),
  (ArithmeticExpr, ArithExpr *)
);

datatype(
  Ast,
  (Expression, Expr *)
);

datatype(
  ExprResult,
  (BooleanResult, bool),
  (NumberResult, double)
);

ArithExpr* aexpr_alloc(ArithExpr ast);
BoolExpr* bexpr_alloc(BoolExpr ast);
Expr* expr_alloc(Expr ast);

double eval_aexpr(ArithExpr* ast);
bool eval_bexpr(BoolExpr* ast);
ExprResult eval(Expr *);

void ast_free_aexpr(ArithExpr* ast);
void ast_free_bexpr(BoolExpr* ast);
void ast_free_expr(Expr* ast);
void ast_free(Ast* ast);
