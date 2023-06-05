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

Ast* ast_alloc(Ast ast) {
  Ast *alloc = malloc(sizeof(Ast));
  ensure_non_null(alloc, "out of space");
  memcpy(alloc, &ast, sizeof(ast));
  return alloc;
}

// Ast* ast_new_binary(Ast *left, BinaryOp op, Ast *right) {
//   Ast *ast = ast_new();

//   ast->type = AstType_BinaryOp;
//   ast->as.binary.left = left;
//   ast->as.binary.op = op;
//   ast->as.binary.right = right;

//   return ast;
// }

// Ast* ast_new_number(double number) {
//   Ast *ast = ast_new();

//   ast->type = AstType_Number;
//   ast->as.number = number;

//   return ast;
// }

double eval(Ast* ast) {
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

void ast_free(Ast* ast) {
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
