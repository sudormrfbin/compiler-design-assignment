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
  Stmt,
  (DisplayStmt, Expr*),
  (ExprStmt, Expr*)
);

datatype(
  Ast,
  (Statement, Stmt *)
);

datatype(
  ExprResult,
  (BooleanResult, bool),
  (NumberResult, double)
);

// TODO: Rename to alloc_* and free_*
ArithExpr* aexpr_alloc(ArithExpr ast);
double eval_aexpr(ArithExpr* ast);
void ast_free_aexpr(ArithExpr* ast);

BoolExpr* bexpr_alloc(BoolExpr ast);
bool eval_bexpr(BoolExpr* ast);
void ast_free_bexpr(BoolExpr* ast);

Expr* expr_alloc(Expr ast);
ExprResult eval_expr(Expr *);
void ast_free_expr(Expr* ast);

Stmt* stmt_alloc(Stmt ast);
void eval_stmt(Stmt* ast);
void ast_free_stmt(Stmt* ast);

Ast* ast_alloc(Ast ast);
void eval(Ast* ast);
void ast_free(Ast* ast);





