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
  Ast,
  (BinaryAExpr, Ast *, BinaryOp, Ast *),
  (UnaryAExpr, UnaryOp, Ast *),
  (Number, double)
);

Ast* ast_alloc(Ast ast);
// Ast* ast_new_binary(Ast* left, BinaryOp op, Ast* right);
// Ast* ast_new_number(double number);

double eval(Ast *);

void ast_free(Ast *);
