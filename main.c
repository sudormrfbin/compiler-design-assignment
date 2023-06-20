#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "main.h"
#include "parser.tab.h"

// Forward declarations and typedefs

extern FILE *yyin;
extern int yylex();
extern int yyparse();
extern int yylineno;

Symbol *symtab[100];
int symtab_index = 0;

void add_symbol(char *name, int value) {
  Symbol *symbol = malloc(sizeof(Symbol));
  symbol->name = name;
  symbol->value = value;
  symtab[symtab_index++] = symbol;
}

int lookup_symbol(char *name) {
  for (int i = symtab_index - 1; i >= 0; i--) {
    if (strcmp(symtab[i]->name, name) == 0) {
      return symtab[i]->value;
    }
  }
  // No match found
  printf("error: Undeclared identifier '%s' used", name);
  exit(1);
}

AstNodeList* mallocNodeList(AstNode* node) {
  AstNodeList* list = malloc(sizeof(AstNodeList));
  list->node = node;
  list->next = NULL;
  return list;
}

void addNodeList(AstNodeList* begin, AstNode* newNode) {
  AstNodeList* end = begin;
  while (end->next != NULL) {
    end = end->next;
  }

  AstNodeList* next = mallocNodeList(newNode);
  end->next = next;
}

// Functions for creating each type of AST node

AstNode *newNode(AstNodeKind kind) {
  AstNode *node = malloc(sizeof(AstNode));
  node->kind = kind;
  return node;
}

AstNode *newNumber(int value) {
  AstNode *node = newNode(nkNumber);
  node->data.number = value;
  return node;
}

AstNode *newIdent(char* ident) {
  AstNode *node = newNode(nkIdent);
  node->data.ident = ident;
  return node;
}

AstNode *newExpr(AstNode* left, char op, AstNode* right) {
  AstNode *node = newNode(nkExpr);
  node->data.expr.left = left;
  node->data.expr.op = op;
  node->data.expr.right = right;
  return node;
}

AstNode *newIf(AstNode* condition, AstNodeList* true_stmts, AstNodeList* else_stmts) {
  AstNode *node = newNode(nkIf);
  node->data.ifStmt.condition = condition;
  node->data.ifStmt.true_stmts = true_stmts;
  node->data.ifStmt.else_stmts = else_stmts;
  return node;
}

AstNode *newWhile(AstNode* condition, AstNodeList* true_stmts) {
  AstNode *node = newNode(nkWhile);
  node->data.whileStmt.condition = condition;
  node->data.whileStmt.true_stmts = true_stmts;
  return node;
}

AstNode *newDisplay(AstNode* expr) {
  AstNode *node = newNode(nkDisplay);
  node->data.display.expr = expr;
  return node;
}

AstNode *newAssign(char* ident, AstNode* expr) {
  AstNode *node = newNode(nkAssign);
  node->data.assign.ident = ident;
  node->data.assign.rhs = expr;
  return node;
}

// Functions for evaluating each type of AST node

int evalExpr(AstNode* expr) {
  if (expr->kind == nkNumber) return expr->data.number;
  else if (expr->kind == nkIdent) return lookup_symbol(expr->data.ident);
  else if (expr->kind == nkExpr) {
    int left = evalExpr(expr->data.expr.left);
    char op = expr->data.expr.op;
    int right = evalExpr(expr->data.expr.right);

    switch (op) {
      case '+': return left + right;
      case '-': return left - right;
      case '*': return left * right;
      case '/': return left / right;
      case '>': return left > right;
      case '<': return left < right;
      case 'G': return left >= right;
      case 'L': return left <= right;
      case 'E': return left == right;
    }
  }
  return -1; // invalid case
}

void evalIfStmt(AstNode* node) {
  int condition = evalExpr(node->data.ifStmt.condition);
  if (condition) {
    evalNodeList(node->data.ifStmt.true_stmts);
  } else {
    evalNodeList(node->data.ifStmt.else_stmts);
  }
}

void evalWhileStmt(AstNode* node) {
  while (evalExpr(node->data.whileStmt.condition)) {
    evalNodeList(node->data.whileStmt.true_stmts);
  }
}

void evalDisplay(AstNode* node) {
  int result = evalExpr(node->data.display.expr);
  printf("%d\n", result);
}

void evalAssign(AstNode* node) {
  int rhs = evalExpr(node->data.assign.rhs);
  // TODO: strdup?
  add_symbol(node->data.assign.ident, rhs);
}

void evalAstNode(AstNode* node) {
  switch (node->kind) {
    case nkNumber: evalExpr(node); break;
    case nkIdent: evalExpr(node); break;
    case nkExpr: evalExpr(node); break;
    case nkIf: evalIfStmt(node); break;
    case nkWhile: evalWhileStmt(node); break;
    case nkDisplay: evalDisplay(node); break;
    case nkAssign: evalAssign(node); break;
  }
}

void evalNodeList(AstNodeList* list) {
  while (list) {
    evalAstNode(list->node);
    list = list->next;
  }
}

int IR_TEMP_IDX = 0;
int IR_LABEL_IDX = 0;

// Consider an if conditional like so:
// ```
// if cond then
//   true_stmts
// else
//   else_stmts
// endif
// rest_of_program
// ```
//
// Then the corresponding 3 address code will be:
//
// ```
// if cond == true goto LTRUE
// else_stmts
// goto LDONE
// LTRUE:
// true_stmts
// LDONE:
// rest_of_program
// ```

// int true_label = IR_LABEL_IDX++;
// printf("if t%d == true goto L%d\n", ir_expr(*condition), true_label);

// if (*else_stmts) {
//   ir_stmt_list(*else_stmts);
// }

// int done_label = IR_LABEL_IDX++;
// printf("goto L%d\n", done_label);

// printf("L%d:\n", true_label);
// ir_stmt_list(*true_stmts);
// printf("L%d:\n", done_label);

// Consider a while statement like so:
// ```
// while cond do
//   true_stmts
// endwhile
// rest_of_program
// ```
//
// Then the corresponding 3 address code will be:
//
// ```
// LBEGIN:
// if cond == true goto LTRUE
// goto LDONE
// LTRUE:
// true_stmts
// goto LBEGIN
// LDONE:
// rest_of_program
// ```
// int begin_label = IR_LABEL_IDX++;
// printf("L%d:\n", begin_label);

// int true_label = IR_LABEL_IDX++;
// printf("if t%d == true goto L%d\n", ir_expr(*condition), true_label);

// int done_label = IR_LABEL_IDX++;
// printf("goto L%d\n", done_label);

// printf("L%d:\n", true_label);
// ir_stmt_list(*true_stmts);
// printf("goto L%d\n", begin_label);

// printf("L%d:\n", done_label);

void yyerror(const char *s) {
  fprintf(stderr, "%d: error: ", yylineno);
  fprintf(stderr, "%s\n", s);
}

int main(int argc, const char **argv) {
  if (argc == 0) {
    fprintf(stderr, "filename is required\n");
    exit(1);
  } else {
    if (!(yyin = fopen(argv[1], "r"))) {
      perror("could not open file");
      return 1;
    }
  }

  yyparse();
}
