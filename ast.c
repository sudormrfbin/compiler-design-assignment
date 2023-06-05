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

ArithExpr* ast_alloc(ArithExpr ast) {
  ArithExpr *alloc = malloc(sizeof(ArithExpr));
  ensure_non_null(alloc, "out of space");
  memcpy(alloc, &ast, sizeof(ast));
  return alloc;
}

double eval(ArithExpr* ast) {
  match(*ast) {
    of(BinaryAExpr, left, op, right) {
      switch (*op) {
        case BinaryOp_Add: return eval(*left) + eval(*right);
        case BinaryOp_Sub: return eval(*left) - eval(*right);
        case BinaryOp_Mul: return eval(*left) * eval(*right);
        case BinaryOp_Div: return eval(*left) / eval(*right);
      }
    }
    of(UnaryAExpr, op, right) {
      switch (*op) {
        case UnaryOp_Minus: return - eval(*right);
      }
    }
    of(Number, num) return *num; 
  }

  return -1; // unreachable
}

void ast_free(ArithExpr* ast) {
  match(*ast) {
    of(BinaryAExpr, left, _, right) {
      ast_free(*left);
      ast_free(*right);
    }
    of(UnaryAExpr, _, right) {
      ast_free(*right);
    }
    of(Number, _) {}; 
  }

  free(ast);
}

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
