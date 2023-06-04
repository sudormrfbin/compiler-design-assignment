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

Ast* newast(int nodetype, Ast *left, Ast *right) {
  Ast *ast = malloc(sizeof(Ast));
  ensure_non_null(ast, "out of space");

  ast->nodetype = nodetype;
  ast->left = left;
  ast->right = right;

  return ast;
}

Ast* newnum(double number) {
  NumVal *val = malloc(sizeof(NumVal));
  ensure_non_null(val, "out of space");

  val->nodetype = 'K';
  val->number = number;

  return (Ast*)val;
}

double eval(Ast* ast) {
  double result;

  switch (ast->nodetype) {
    case 'K': result = ((NumVal*)ast)->number; break;
    case '+': result = eval(ast->left) + eval(ast->right); break;
    case '-': result = eval(ast->left) - eval(ast->right); break;
    case '*': result = eval(ast->left) * eval(ast->right); break;
    case '/': result = eval(ast->left) / eval(ast->right); break;
    case '|': result = eval(ast->left); if (result < 0) result = -result; break;
    case 'M': result = -eval(ast->left); break;
    default: printf("intenral error: bad node %c\n", ast->nodetype);
  }

  return result;
}

void treefree(Ast* ast) {
  switch (ast->nodetype) {
    // two subtrees
    case '+':
    case '-':
    case '*':
    case '/': 
      treefree(ast->right);
    // one subtree
    case '|':
    case 'M':
      treefree(ast->left);
    case 'K':
      free(ast);
      break;
    default: printf("internal error: free bad node %c\n", ast->nodetype);
  }
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
