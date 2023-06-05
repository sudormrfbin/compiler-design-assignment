#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "datatype99.h"
#include "parser.tab.h"

void ensure_non_null(void *ptr, char *msg) {
  if (!ptr) {
    yyerror(msg);
    exit(1);
  }
}

ArithExpr* aexpr_alloc(ArithExpr ast) {
  ArithExpr *alloc = malloc(sizeof(ast));
  ensure_non_null(alloc, "out of space");
  memcpy(alloc, &ast, sizeof(ast));
  return alloc;
}

BoolExpr* bexpr_alloc(BoolExpr ast) {
  BoolExpr *alloc = malloc(sizeof(ast));
  ensure_non_null(alloc, "out of space");
  memcpy(alloc, &ast, sizeof(ast));
  return alloc;
}

Expr* expr_alloc(Expr ast) {
  Expr *alloc = malloc(sizeof(ast));
  ensure_non_null(alloc, "out of space");
  memcpy(alloc, &ast, sizeof(ast));
  return alloc;
}

/* ------------------- Tree Walking Evaluation ---------------------- */

double eval_aexpr(ArithExpr* ast) {
  match(*ast) {
    of(BinaryAExpr, left, op, right) {
      switch (*op) {
        case BinaryOp_Add: return eval_aexpr(*left) + eval_aexpr(*right);
        case BinaryOp_Sub: return eval_aexpr(*left) - eval_aexpr(*right);
        case BinaryOp_Mul: return eval_aexpr(*left) * eval_aexpr(*right);
        case BinaryOp_Div: return eval_aexpr(*left) / eval_aexpr(*right);
      }
    }
    of(UnaryAExpr, op, right) {
      switch (*op) {
        case UnaryOp_Minus: return - eval_aexpr(*right);
      }
    }
    of(Number, num) return *num; 
  }

  return -1; // unreachable
}

bool eval_bexpr(BoolExpr* ast) {
  match (*ast) {
    of(RelationalArithExpr, left, relop, right) {
      switch (*relop) {
        case RelationalEqual: return eval_aexpr(*left) == eval_aexpr(*right);
        case Greater:         return eval_aexpr(*left) >  eval_aexpr(*right);
        case GreaterOrEqual:  return eval_aexpr(*left) >= eval_aexpr(*right);
        case Less:            return eval_aexpr(*left) <  eval_aexpr(*right);
        case LessOrEqual:     return eval_aexpr(*left) <= eval_aexpr(*right);
      }
    }
    of(LogicalBoolExpr, left, logicalop, right) {
      switch (*logicalop) {
        case And: return eval_bexpr(*left) && eval_bexpr(*right);
        case Or: return eval_bexpr(*left) || eval_bexpr(*right);
        case LogicalEqual: return eval_bexpr(*left) == eval_bexpr(*right);
      }
    }
    of(NegatedBoolExpr, bexpr) return !eval_bexpr(*bexpr);
    of(Boolean, boolean) return *boolean;
  }

  return false; // unreachable
}

ExprResult eval(Expr* expr) {
  match (*expr) {
    of(BooleanExpr, bexpr) return BooleanResult(eval_bexpr(*bexpr));
    of(ArithmeticExpr, aexpr) return NumberResult(eval_aexpr(*aexpr));
  }

  return BooleanResult(false); // unreachable
}

/* ------------------------ AST Memory Freeing -------------------------- */

void ast_free_aexpr(ArithExpr* ast) {
  match(*ast) {
    of(BinaryAExpr, left, _, right) {
      ast_free_aexpr(*left);
      ast_free_aexpr(*right);
    }
    of(UnaryAExpr, _, right) {
      ast_free_aexpr(*right);
    }
    of(Number, _) {}; 
  }

  free(ast);
}

void ast_free_bexpr(BoolExpr* ast) {
  match (*ast) {
    of(RelationalArithExpr, left, _, right) {
      ast_free_aexpr(*left);
      ast_free_aexpr(*right);
    }
    of(LogicalBoolExpr, left, _, right) {
      ast_free_bexpr(*left);
      ast_free_bexpr(*right);
    }
    of(NegatedBoolExpr, bexpr) ast_free_bexpr(*bexpr);
    of(Boolean, _) {}
  }

  free(ast);
}

void ast_free_expr(Expr* ast) {
  match (*ast) {
    of(BooleanExpr, bexpr) ast_free_bexpr(*bexpr);
    of(ArithmeticExpr, aexpr) ast_free_aexpr(*aexpr);
  }

  free(ast);
}

void ast_free(Ast* ast) {
  match (*ast) {
    of(Expression, expr) ast_free_expr(*expr);
  }
}

/* ------------------------------------------------------------------- */

void yyerror(const char *s, ...) {
  va_list ap;
  va_start(ap, s);

  fprintf(stderr, "%d: error: ", yylineno);
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
}

int main() {
  printf("> ");
  return yyparse();
}
