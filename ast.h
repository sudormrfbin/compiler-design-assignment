#include "datatype99.h"

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

datatype(
  ArithExpr,
  (BinaryAExpr, ArithExpr *, BinaryOp, ArithExpr *),
  (UnaryAExpr, UnaryOp, ArithExpr *),
  (Number, double)
);

ArithExpr* ast_alloc(ArithExpr ast);

double eval(ArithExpr *);

void ast_free(ArithExpr *);
