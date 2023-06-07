#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "datatype99.h"
#include "parser.tab.h"

extern SymbolTable* symtab;
extern FILE* yyin;
extern StatementList* parse_result;

// Prints 2 * level number of spaces
void print_indent(int level) {
  for (int i=1; i<=level; i++) printf("  "); 
}

// Like printf, but indents the string by the given indent level.
void iprintf(int indent, const char *s, ...) {
  va_list ap;
  va_start(ap, s);

  print_indent(indent);
  vprintf(s, ap);
}

void ensure_non_null(void *ptr, char *msg) {
  if (!ptr) {
    fprintf(stderr, "%s", msg);
    exit(1);
  }
}

void unreachable(const char *func_name) {
  fprintf(stderr, "reached an unreachable block in function '%s'", func_name);
  exit(1);
}

/* ----------------------------------------------------------------- */

#define ALLOC_NODE(type, node) \
    type* node##_alloc(type ast) { \
        type* alloc = malloc(sizeof(ast)); \
        ensure_non_null(alloc, "out of space"); \
        memcpy(alloc, &ast, sizeof(ast)); \
        return alloc; \
    }

ALLOC_NODE(ArithExpr, aexpr)
ALLOC_NODE(BoolExpr, bexpr)
ALLOC_NODE(StrExpr, sexpr)
ALLOC_NODE(Expr, expr)
ALLOC_NODE(Stmt, stmt)

/* ------------------------ ArithmeticExpression ------------------------ */

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

  unreachable("eval_aexpr");
  return -1;
}

void print_aexpr(ArithExpr* ast, int ind) {
  match(*ast) {
    of(BinaryAExpr, left, op, right) {
      iprintf(ind, "BinaryExpression(\n");
      ind++;
      print_aexpr(*left, ind);

      iprintf(ind, "Op(");
      switch (*op) {
        case BinaryOp_Add: printf("+"); break;
        case BinaryOp_Sub: printf("-"); break;
        case BinaryOp_Mul: printf("*"); break;
        case BinaryOp_Div: printf("/"); break;
      }
      printf(")\n");

      print_aexpr(*right, ind);
      ind--;
      iprintf(ind, ")\n");
    }
    of(UnaryAExpr, op, right) {
      iprintf(ind, "UnaryExpression(\n");
      ind++;

      iprintf(ind, "Op(");
      switch (*op) {
        case UnaryOp_Minus: printf("-"); break;
      }
      printf(")\n");

      print_aexpr(*right, ind);
      ind--;
      iprintf(ind, ")\n");
    }
    of(Number, num) iprintf(ind, "Number(%g)\n", *num); 
  }

  ind--;
}

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

/* -------------------------- BoolExpression --------------------------- */


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

  unreachable("eval_bexpr");
  return false;
}

void print_bexpr(BoolExpr* ast, int ind) {
  match (*ast) {
    of(RelationalArithExpr, left, relop, right) {
      iprintf(ind, "RelationalExpression(\n");
      ind++;
      print_aexpr(*left, ind);

      iprintf(ind, "Op(");
      switch (*relop) {
        case RelationalEqual: printf("=="); break;
        case Greater:         printf(">"); break;
        case GreaterOrEqual:  printf(">="); break;
        case Less:            printf("<"); break;
        case LessOrEqual:     printf("<="); break;
      }
      printf(")\n");
      print_aexpr(*right, ind);
      ind--;
    }
    of(LogicalBoolExpr, left, logicalop, right) {
      iprintf(ind, "LogicalExpression(\n");
      ind++;
      print_bexpr(*left, ind);

      iprintf(ind, "Op(");
      switch (*logicalop) {
        case And: printf("&&"); break;
        case Or: printf("||"); break;
        case LogicalEqual: printf("=="); break;
      }
      printf(")\n");

      print_bexpr(*right, ind);
      ind--;
    }
    of(NegatedBoolExpr, bexpr) {
      iprintf(ind, "Op(!)\n");
      print_bexpr(*bexpr, ind);
    }
    of(Boolean, boolean) {
      iprintf(ind, "Boolean(%s)\n", *boolean ? "true" : "false");
    };
  }
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

/* --------------------------- StringExpression --------------------------- */

char* eval_sexpr(StrExpr* ast) {
  match (*ast) {
    of(StringConcat, first, second) {
      char* left = eval_sexpr(*first);
      char* right = eval_sexpr(*second);

      int size = strlen(left) + strlen(right) + 1;
      char* new = malloc(size);
      snprintf(new, size, "%s%s", left, right);
      return new;
    }
    of(String, str) return *str;
  }

  unreachable("eval_sexpr");
  return NULL;
}

void print_sexpr(StrExpr* ast, int ind) {
  match (*ast) {
    of(StringConcat, first, second) {
      iprintf(ind, "StringConcat(\n");
      print_sexpr(*first, ind + 1);
      print_sexpr(*second, ind + 1);
      iprintf(ind, ")\n");
    }
    of(String, str) iprintf(ind, "String(\"%s\")", *str);
  }
}

void ast_free_sexpr(StrExpr* ast) {
  match (*ast) {
    of(StringConcat, first, second) {
      ast_free_sexpr(*first);
      ast_free_sexpr(*second);
    }
    of(String, str) free(*str);
  }
}

/* ----------------------------- Expression ----------------------------- */

ExprResult eval_expr(Expr* expr) {
  match (*expr) {
    of(BooleanExpr, bexpr) return BooleanResult(eval_bexpr(*bexpr));
    of(ArithmeticExpr, aexpr) return NumberResult(eval_aexpr(*aexpr));
    of(StringExpr, sexpr) return StringResult(eval_sexpr(*sexpr));
    of(IdentExpr, ident) {
      ExprResult* value = symbol_get(symtab, *ident);
      if (!value) {
        fprintf(stderr, "Runtime error: undefined variable '%s'\n", *ident);
        exit(1);
      }
      return *value;
    }
  }

  unreachable("eval_expr");
  return BooleanResult(false);
}

void print_expr(Expr* ast, int ind) {
  match (*ast) {
    of(BooleanExpr, bexpr) {
      iprintf(ind, "BooleanExpression(\n");
      print_bexpr(*bexpr, ind + 1);
      iprintf(ind, ")\n");
    }
    of(ArithmeticExpr, aexpr) {
      iprintf(ind, "ArithmeticExpression(\n");
      print_aexpr(*aexpr, ind + 1);
      iprintf(ind, ")\n");
    }
    of(StringExpr, sexpr) {
      iprintf(ind, "StringExpression(\n");
      print_sexpr(*sexpr, ind + 1);
      iprintf(ind, ")\n");
    }
    of(IdentExpr, ident) {
      iprintf(ind, "Variable(\"%s\")\n", *ident);
    }
  }
}

void ast_free_expr(Expr* ast) {
  match (*ast) {
    of(BooleanExpr, bexpr) ast_free_bexpr(*bexpr);
    of(ArithmeticExpr, aexpr) ast_free_aexpr(*aexpr);
    of(StringExpr, sexpr) ast_free_sexpr(*sexpr);
    of(IdentExpr, ident) free(*ident);
  }

  free(ast);
}

/* ----------------------------- Statement ----------------------------- */

void eval_stmt(Stmt* stmt) {
  match (*stmt) {
    of(DisplayStmt, expr) {
      ExprResult result = eval_expr(*expr);
      match(result) {
        of(BooleanResult, boolean) printf("%s\n", *boolean ? "true" : "false");
        of(NumberResult, number) printf("%g\n", *number);
        of(StringResult, string) printf("%s\n", *string);
      }
    }
    of(ExprStmt, expr) eval_expr(*expr); 
    of(AssignStmt, ident, value) {
      symbol_add(&symtab, *ident, eval_expr(*value));
    }
    of(IfStmt, condition, true_stmts, else_stmts) {
      if (eval_bexpr(*condition)) {
        eval_stmt_list(*true_stmts);
      } else {
        eval_stmt_list(*else_stmts);
      }
    }
  }
}

void print_stmt(Stmt *ast, int ind) {
  match (*ast) {
    of(DisplayStmt, expr) {
      iprintf(ind, "DisplayStatement(\n");
      print_expr(*expr, ind + 1);
      iprintf(ind, ")\n");
    }
    of(ExprStmt, expr) {
      iprintf(ind, "ExpressionStatement(\n");
      print_expr(*expr, ind + 1);
      iprintf(ind, ")\n");
    }; 
    of(AssignStmt, ident, value) {
      iprintf(ind, "AssignmentStatement(\n");
      iprintf(ind + 1, "Ident(%s)", *ident);
      print_expr(*value, ind + 1);
      iprintf(ind, ")\n");
    }; 
    of(IfStmt, condition, true_stmts, else_stmts) {
      iprintf(ind, "IfStatement(\n");

      iprintf(ind + 1, "Condition(\n");
      print_bexpr(*condition, ind + 2);
      iprintf(ind + 1, ")\n");

      iprintf(ind + 1, "TrueStatements(\n");
      print_stmt_list(*true_stmts, ind + 2);
      iprintf(ind + 1, ")\n");

      if (*else_stmts) {
        iprintf(ind + 1, "ElseStatements(\n");
        print_stmt_list(*else_stmts, ind + 2);
        iprintf(ind + 1, ")\n");
      }

      iprintf(ind, ")\n");
    }
  }
}

void ast_free_stmt(Stmt* ast) {
  match (*ast) {
    of(DisplayStmt, expr) ast_free_expr(*expr);
    of(ExprStmt, expr) ast_free_expr(*expr);
    of(AssignStmt, ident, value) {
      free(*ident);
      ast_free_expr(*value);
    }
    of(IfStmt, condition, true_stmts, else_stmts) {
      ast_free_bexpr(*condition);
      stmt_list_free(*true_stmts);
      stmt_list_free(*else_stmts);
    }
  }
}

/* -------------------------- StatementList -------------------------- */

StatementList* stmt_list_alloc() {
  StatementList* alloc = malloc(sizeof(StatementList));
  ensure_non_null(alloc, "out of space");
  alloc->next = NULL;
  alloc->prev = NULL;
  alloc->value = NULL;

  return alloc;
}

void stmt_list_add(StatementList** start, Stmt* stmt) {
  if (!(*start)) {
    *start = stmt_list_alloc();
    (*start)->value = stmt;
    return;
  } else {
    StatementList* end = *start;
    while (end->next) end = end->next;

    StatementList* new = stmt_list_alloc();
    new->value = stmt;
    new->prev = end;
    end->next = new;
    return;
  }
}

void eval_stmt_list(StatementList* start) {
  StatementList* curr = start;
  while (curr) {
    eval_stmt(curr->value);
    curr = curr->next;
  }
}

void print_stmt_list(StatementList* start, int ind) {
  StatementList* curr = start;
  while (curr) {
    iprintf(ind, "Statement(\n");
    print_stmt(curr->value, ind + 1);
    iprintf(ind, ")\n");

    curr = curr->next;
  }
}

void stmt_list_free(StatementList* start) {
  StatementList* curr = start;
  while (curr) {
    ast_free_stmt(curr->value);
    curr = curr->next;
  }
}

/* --------------------------- Symbol table --------------------------- */

Symbol* symbol_alloc(char* name, ExprResult value) {

  // TODO: Add commands for printing tokens, ast, 3 address code, symtab
  Symbol* alloc = malloc(sizeof(Symbol));
  // TODO: ensure_non_null
  alloc->name = name;
  alloc->value = value;
  alloc->next = NULL;
  return alloc;
}

void symbol_add(SymbolTable** head, char* name, ExprResult value) {
  if (!(*head)) {
    *head = symbol_alloc(name, value);
    return;
  } else {
    Symbol* curr = *head;
    Symbol* end;

    do {
      if (strcmp(curr->name, name) == 0) {
        curr->value = value;
        return;
      }
      end = curr;
    } while ((curr = curr->next));

    Symbol* new = symbol_alloc(name, value);
    end->next = new;
  }
}

ExprResult* symbol_get(SymbolTable* head, char* name) {
  Symbol* curr = head;

  while (curr) {
    if (strcmp(curr->name, name) == 0) {
      return &(curr->value);
    }
    curr = curr->next;
  }

  return NULL;
}

void print_symtab(SymbolTable* head) {
  Symbol* curr = head;

  while (curr) {
    printf("%s = ", curr->name);
    match(curr->value) {
      of(BooleanResult, boolean) printf("%s\n", *boolean ? "true" : "false");
      of(NumberResult, number) printf("%g\n", *number);
      of(StringResult, string) printf("%s\n", *string);
    }
    curr = curr->next;
  }
}

/* ---------------------------------------------------------------------- */

void yyerror(const char *s, ...) {
  va_list ap;
  va_start(ap, s);

  fprintf(stderr, "%d: error: ", yylineno);
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
}

int main(int argc, char **argv) {
  // If a file is given as command line argument, set input
  // to file contents.
  if (argc > 1) {
    if (!(yyin = fopen(argv[1], "r"))) {
      perror(argv[1]);
      return 1;
    }
  }

  // TODO: Add commands for printing tokens, ast, 3 address code, symtab

  yyparse();
  // print_stmt_list(parse_result, 0);
  eval_stmt_list(parse_result);
  stmt_list_free(parse_result);

  return 0;
}
