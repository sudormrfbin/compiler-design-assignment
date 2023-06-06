#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "datatype99.h"
#include "parser.tab.h"

extern FILE* yyin;

void ensure_non_null(void *ptr, char *msg) {
  if (!ptr) {
    yyerror(NULL, msg);
    exit(1);
  }
}

#define ALLOC_NODE(type, node) \
    type* node##_alloc(type ast) { \
        type* alloc = malloc(sizeof(ast)); \
        ensure_non_null(alloc, "out of space"); \
        memcpy(alloc, &ast, sizeof(ast)); \
        return alloc; \
    }

ALLOC_NODE(ArithExpr, aexpr)
ALLOC_NODE(BoolExpr, bexpr)
ALLOC_NODE(Expr, expr)
ALLOC_NODE(Stmt, stmt)
ALLOC_NODE(Ast, ast)

Statements* statements_alloc() {
  Statements* alloc = malloc(sizeof(Statements));
  ensure_non_null(alloc, "out of space");
  alloc->next = NULL;
  alloc->prev = NULL;
  alloc->value = NULL;

  return alloc;
}

void statements_add_stmt(Statements* start, Stmt* stmt) {
  if (!start) {
    start = statements_alloc();
    start->value = stmt;
    return;
  } else {
    Statements* end = start;
    while (end->next) end = end->next;

    Statements* new = statements_alloc();
    new->value = stmt;
    new->prev = end;
    end->next = new;
    return;
  }
}

void eval_statements(Statements* start) {
  Statements* curr = start;
  while (curr) {
    eval_stmt(curr->value);
    curr = curr->next;
  }
}

void statements_free(Statements* start) {
  Statements* curr = start;
  while (curr) {
    ast_free_stmt(curr->value);
    curr = curr->next;
  }
}

/* ------------------- Tree Walking Evaluation ---------------------- */

// TODO: exit with error if reaching unreachable branch

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

ExprResult eval_expr(Expr* expr) {
  match (*expr) {
    of(BooleanExpr, bexpr) return BooleanResult(eval_bexpr(*bexpr));
    of(ArithmeticExpr, aexpr) return NumberResult(eval_aexpr(*aexpr));
  }

  return BooleanResult(false); // unreachable
}

void eval_stmt(Stmt* stmt) {
  match (*stmt) {
    of(DisplayStmt, expr) {
      ExprResult result = eval_expr(*expr);
      match(result) {
        of(BooleanResult, boolean) printf("%s\n", *boolean ? "true" : "false");
        of(NumberResult, number) printf("%g\n", *number);
      }
    }
    of(ExprStmt, expr) eval_expr(*expr); 
    of(IfStmt, condition, true_stmts) {
      if (eval_bexpr(*condition)) {
        eval_statements(*true_stmts);
      }
    }
  }
}

void eval(Ast* ast) {
  match (*ast) {
    of(Statement, stmt) eval_stmt(*stmt);
  }
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

void ast_free_stmt(Stmt* ast) {
  match (*ast) {
    of(DisplayStmt, expr) ast_free_expr(*expr);
    of(ExprStmt, expr) ast_free_expr(*expr);
    of(IfStmt, condition, true_stmts) {
      ast_free_bexpr(*condition);
      statements_free(*true_stmts);
    }
  }
}

void ast_free(Ast* ast) {
  match (*ast) {
    of(Statement, stmt) ast_free_stmt(*stmt);
  }
}

/* ------------------------------------------------------------------- */

void yyerror(Statements* parse_result, const char *s, ...) {
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

  Statements* parse_result = NULL;
  yyparse(parse_result);
  eval_statements(parse_result);

  return 0;
}
