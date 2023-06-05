#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "parser.tab.h"

void ensure_non_null(void *ptr, char *msg) {
  if (!ptr) {
    yyerror(msg);
    exit(1);
  }
}

Ast* ast_new() {
  Ast *ast = malloc(sizeof(Ast));
  ensure_non_null(ast, "out of space");
  return ast;
}

Ast* ast_new_binary(Ast *left, BinaryOp op, Ast *right) {
  Ast *ast = ast_new();

  ast->type = AstType_BinaryOp;
  ast->as.binary.left = left;
  ast->as.binary.op = op;
  ast->as.binary.right = right;

  return ast;
}

Ast* ast_new_number(double number) {
  Ast *ast = ast_new();

  ast->type = AstType_Number;
  ast->as.number = number;

  return ast;
}

double eval(Ast* ast) {
  switch (ast->type) {
    case AstType_BinaryOp: {
      AstNodeBinary node = ast->as.binary;
      switch (ast->as.binary.op) {
        case BinaryOp_Add: return eval(node.left) + eval(node.right);
        case BinaryOp_Sub: return eval(node.left) - eval(node.right);
        case BinaryOp_Mul: return eval(node.left) * eval(node.right);
        case BinaryOp_Div: return eval(node.left) / eval(node.right);
      }
      break;
    }
    case AstType_Number: return ast->as.number;
  }

  return -1; // unreachable
}

void ast_free(Ast* ast) {
  switch (ast->type) {
    case AstType_BinaryOp:
      ast_free(ast->as.binary.left);
      ast_free(ast->as.binary.right);
      break;
    case AstType_Number:
        break;
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
