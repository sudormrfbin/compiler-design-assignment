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

char* concat_str(char* left, char* right) {
  int size = strlen(left) + strlen(right) + 1;
  char* new = malloc(size);
  snprintf(new, size, "%s%s", left, right);
  return new;
}

void runtime_error(const char *s, ...) {
  va_list ap;
  va_start(ap, s);

  fprintf(stderr, "runtime error: ");
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
  exit(1);
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
    type* alloc_##node(type ast) { \
        type* alloc = malloc(sizeof(ast)); \
        ensure_non_null(alloc, "out of space"); \
        memcpy(alloc, &ast, sizeof(ast)); \
        return alloc; \
    }

ALLOC_NODE(ArithExpr, aexpr)
ALLOC_NODE(BoolExpr, bexpr)
ALLOC_NODE(StrExpr, sexpr)
ALLOC_NODE(LiteralExpr, literal_expr)
ALLOC_NODE(IdentExpr, ident_expr)
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

void free_aexpr(ArithExpr* ast) {
  match(*ast) {
    of(BinaryAExpr, left, _, right) {
      free_aexpr(*left);
      free_aexpr(*right);
    }
    of(UnaryAExpr, _, right) {
      free_aexpr(*right);
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

void free_bexpr(BoolExpr* ast) {
  match (*ast) {
    of(RelationalArithExpr, left, _, right) {
      free_aexpr(*left);
      free_aexpr(*right);
    }
    of(LogicalBoolExpr, left, _, right) {
      free_bexpr(*left);
      free_bexpr(*right);
    }
    of(NegatedBoolExpr, bexpr) free_bexpr(*bexpr);
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
      return concat_str(left, right);
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

void free_sexpr(StrExpr* ast) {
  match (*ast) {
    of(StringConcat, first, second) {
      free_sexpr(*first);
      free_sexpr(*second);
    }
    of(String, str) free(*str);
  }
}

/* ----------------------------- LiteralExpression ----------------------------- */

ExprResult eval_literal_expr(LiteralExpr* expr) {
  match (*expr) {
    of(BooleanExpr, bexpr) return BooleanResult(eval_bexpr(*bexpr));
    of(ArithmeticExpr, aexpr) return NumberResult(eval_aexpr(*aexpr));
    of(StringExpr, sexpr) return StringResult(eval_sexpr(*sexpr));
  }

  unreachable("eval_literal_expr");
  return BooleanResult(false);
}

void print_literal_expr(LiteralExpr* ast, int ind) {
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
  }
}

void free_literal_expr(LiteralExpr* ast) {
  match (*ast) {
    of(BooleanExpr, bexpr) free_bexpr(*bexpr);
    of(ArithmeticExpr, aexpr) free_aexpr(*aexpr);
    of(StringExpr, sexpr) free_sexpr(*sexpr);
  }

  free(ast);
}

/* ------------------------ IdentifierExpression ------------------------ */

ExprResult eval_binary_ident_expr(char* ident, IdentBinaryOp op, LiteralExpr* expr) {
  ExprResult lhs = symbol_get(symtab, ident);
  ExprResult rhs = eval_literal_expr(expr);

  match (lhs) {
    of(BooleanResult, bool1) {
      match (rhs) {
        of(BooleanResult, bool2) {
          switch (op) {
            case IdentBOp_EqEq: return BooleanResult(*bool1 == *bool2);
            case IdentBOp_And: return BooleanResult(*bool1 && *bool2);
            case IdentBOp_Or: return BooleanResult(*bool1 || *bool2);
            default: runtime_error("unsupported boolean operation");
          }
        }
        otherwise {
          switch (op) {
            case IdentBOp_EqEq: return BooleanResult(false);
            default: runtime_error("unsupported boolean operation");
          }
        }
      }
    }
    of(StringResult, str1) {
      match (rhs) {
        of(StringResult, str2) {
          switch (op) {
            case IdentBOp_Plus: return StringResult(concat_str(*str1, *str2));
            case IdentBOp_EqEq: return BooleanResult(strcmp(*str1, *str2));
            default: runtime_error("unsupported string operation");
          }
        }
        otherwise {
          switch (op) {
            case IdentBOp_EqEq: return BooleanResult(false);
            default: runtime_error("unsupported string operation");
          }
        }
      }
    }
    of(NumberResult, num1) {
      match (rhs) {
        of(NumberResult, num2) {
          switch (op) {
            case IdentBOp_Plus: return NumberResult(*num1 + *num2);
            case IdentBOp_Minus: return NumberResult(*num1 - *num2);
            case IdentBOp_Star: return NumberResult(*num1 * *num2);
            case IdentBOp_Slash: return NumberResult(*num1 / *num2);

            case IdentBOp_Gt: return BooleanResult(*num1 > *num2);
            case IdentBOp_Gte: return BooleanResult(*num1 >= *num2);
            case IdentBOp_Lt: return BooleanResult(*num1 < *num2);
            case IdentBOp_Lte: return BooleanResult(*num1 <= *num2);
            case IdentBOp_EqEq: return BooleanResult(*num1 == *num2);
            default: runtime_error("unsupported number operation");
          }
        }
        otherwise {
          switch (op) {
            case IdentBOp_EqEq: return BooleanResult(false);
            default: runtime_error("unsupported number operation");
          }
        }
      }
    }
  }
  unreachable("eval_binary_ident_expr");
  return BooleanResult(false);
}

ExprResult eval_ident_expr(IdentExpr* expr) {
  match (*expr) {
    of(IdentBinaryExpr, ident, op, expr) return eval_binary_ident_expr(*ident, *op, *expr);
    of(IdentUnaryExpr, op, ident) {
      ExprResult value = symbol_get(symtab, *ident);
      switch (*op) {
        case IdentUOp_Minus: {
          match (value) {
            of(NumberResult, num) return NumberResult(- (*num));
            otherwise runtime_error("unsupported variable type for number negation");
          }
          break;
        }
        case IdentUOp_Exclamation:
          match (value) {
            of(BooleanResult, boolean) return BooleanResult(!boolean);
            otherwise runtime_error("unsupported variable type for boolean negation");
          }
          break;
      }
    }
    of(Identifier, ident) return symbol_get(symtab, *ident);
  }

  unreachable("eval_ident_expr");
  return BooleanResult(false);
}

void print_ident_expr(IdentExpr* ast, int ind) {
  match (*ast) {
    of(IdentBinaryExpr, ident, op, expr) {
      iprintf(ind, "BinaryExpression(\n");
      ind++;
      iprintf(ind, "Variable(\"%s\")\n", *ident);

      iprintf(ind, "Op(");
      switch (*op) {
        case IdentBOp_Plus:  printf("+");  break;
        case IdentBOp_Minus: printf("-");  break;
        case IdentBOp_Star:  printf("*");  break;
        case IdentBOp_Slash: printf("/");  break;
        case IdentBOp_And:   printf("&&"); break;
        case IdentBOp_Or:    printf("||"); break;
        case IdentBOp_EqEq:  printf("=="); break;
        case IdentBOp_Gt:    printf(">");  break;
        case IdentBOp_Gte:   printf(">="); break;
        case IdentBOp_Lt:    printf("<");  break;
        case IdentBOp_Lte:   printf("<="); break;
      }
      printf(")\n");

      print_literal_expr(*expr, ind);
      ind--;
      iprintf(ind, ")\n");
    }
    of(IdentUnaryExpr, op, ident) {
      iprintf(ind, "UnaryExpression(\n");
      ind++;

      iprintf(ind, "Op(");
      switch (*op) {
        case IdentUOp_Minus: printf("-"); break;
        case IdentUOp_Exclamation: printf("!"); break;
      }
      printf(")\n");

      iprintf(ind, "Variable(\"%s\")\n", *ident);
      ind--;
      iprintf(ind, ")\n");
    }
    of(Identifier, ident) iprintf(ind, "Variable(\"%s\")\n", *ident);
  }
}

void free_ident_expr(IdentExpr* ast) {
  match (*ast) {
    of(IdentBinaryExpr, ident, _, expr) {
      free(*ident);
      free_literal_expr(*expr);
    }
    of(IdentUnaryExpr, _, ident) free(*ident);
    of(Identifier, ident) free(*ident);
  }
}

/* ----------------------------- Expression ----------------------------- */

ExprResult eval_expr(Expr* expr) {
  match (*expr) {
    of(LiteralExpression, lexpr) return eval_literal_expr(*lexpr);
    of(IdentExpression, iexpr) return eval_ident_expr(*iexpr);
  }

  unreachable("eval_expr");
  return BooleanResult(false);
}

void print_expr(Expr* ast, int ind) {
  match (*ast) {
    of(LiteralExpression, lexpr) print_literal_expr(*lexpr, ind);
    of(IdentExpression, iexpr) print_ident_expr(*iexpr, ind);
  }
}

void free_expr(Expr* ast) {
  match (*ast) {
    of(LiteralExpression, lexpr) free_literal_expr(*lexpr);
    of(IdentExpression, iexpr) free_ident_expr(*iexpr);
  }
}

/* ----------------------------- Statement ----------------------------- */

bool eval_to_condition(Expr* expr) {
  ExprResult evaled = eval_expr(expr);
  match (evaled) {
    of(BooleanResult, boolean) return *boolean;
    otherwise { runtime_error("if condition must evaluate to a boolean"); }
  }

  unreachable("eval_to_condition");
  return false;
}

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
      add_symbol(&symtab, *ident, eval_expr(*value));
    }
    of(IfStmt, condition, true_stmts, else_if, else_stmts) {
      if (eval_to_condition(*condition)) {
        eval_stmt_list(*true_stmts);
      } else {
        bool else_if_did_exec = eval_else_if(*else_if);
        if (!else_if_did_exec) {
          eval_stmt_list(*else_stmts);
        }
      }
    }
    of(WhileStmt, condition, true_stmts) {
      while (eval_to_condition(*condition)) {
        eval_stmt_list(*true_stmts);
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
    of(IfStmt, condition, true_stmts, else_if, else_stmts) {
      iprintf(ind, "IfStatement(\n");

      iprintf(ind + 1, "Condition(\n");
      print_expr(*condition, ind + 2);
      iprintf(ind + 1, ")\n");

      iprintf(ind + 1, "TrueStatements(\n");
      print_stmt_list(*true_stmts, ind + 2);
      iprintf(ind + 1, ")\n");

      if (*else_if) {
        print_else_if(*else_if, ind + 1);
      }

      if (*else_stmts) {
        iprintf(ind + 1, "ElseStatements(\n");
        print_stmt_list(*else_stmts, ind + 2);
        iprintf(ind + 1, ")\n");
      }

      iprintf(ind, ")\n");
    }
    of(WhileStmt, condition, true_stmts) {
      iprintf(ind, "WhileStatement(\n");

      iprintf(ind + 1, "Condition(\n");
      print_expr(*condition, ind + 2);
      iprintf(ind + 1, ")\n");

      iprintf(ind + 1, "TrueStatements(\n");
      print_stmt_list(*true_stmts, ind + 2);
      iprintf(ind + 1, ")\n");

      iprintf(ind, ")\n");
    }
  }
}

void free_stmt(Stmt* ast) {
  match (*ast) {
    of(DisplayStmt, expr) free_expr(*expr);
    of(ExprStmt, expr) free_expr(*expr);
    of(AssignStmt, ident, value) {
      free(*ident);
      free_expr(*value);
    }
    of(IfStmt, condition, true_stmts, else_if, else_stmts) {
      free_expr(*condition);
      free_stmt_list(*true_stmts);
      free_else_if(*else_if);
      free_stmt_list(*else_stmts);
    }
    of(WhileStmt, condition, true_stmts) {
      free_expr(*condition);
      free_stmt_list(*true_stmts);
    }
  }
}

/* -------------------------- ElseIfStatement ------------------------ */

ElseIfStatement* alloc_else_if(Condition* cond, TrueStatements* stmts) {
  ElseIfStatement* alloc = malloc(sizeof(ElseIfStatement));
  ensure_non_null(alloc, "out of space");
  alloc->condition = cond;
  alloc->true_stmts = stmts;
  alloc->next = NULL;

  return alloc;
}

void add_else_if(ElseIfStatement** start, Condition* cond, TrueStatements* stmts) {
  ElseIfStatement* new = alloc_else_if(cond, stmts);

  if (!(*start)) {
    *start = new;
  } else {
    ElseIfStatement* end = *start;
    while (end->next) end = end->next;
    end->next = new;
  }
}

bool eval_else_if(ElseIfStatement* stmt) {
  while (stmt) {
    if (eval_to_condition(stmt->condition)) {
      eval_stmt_list(stmt->true_stmts);
      return true;
    }
    stmt = stmt->next;
  }

  return false;
}

void free_else_if(ElseIfStatement* stmt) {
  while (stmt) {
    free_expr(stmt->condition);
    free_stmt_list(stmt->true_stmts);
    stmt = stmt->next;
  }
}

void print_else_if(ElseIfStatement* stmt, int ind) {
  while (stmt) {
    iprintf(ind, "ElseIfStatement(\n");

    iprintf(ind + 1, "Condition(\n");
    print_expr(stmt->condition, ind + 2);
    iprintf(ind + 1, ")\n");

    iprintf(ind + 1, "TrueStatements(\n");
    print_stmt_list(stmt->true_stmts, ind + 2);
    iprintf(ind + 1, ")\n");

    iprintf(ind, ")\n");

    stmt = stmt->next;
  }
}

/* -------------------------- StatementList -------------------------- */

StatementList* alloc_stmt_list() {
  StatementList* alloc = malloc(sizeof(StatementList));
  ensure_non_null(alloc, "out of space");
  alloc->next = NULL;
  alloc->prev = NULL;
  alloc->value = NULL;

  return alloc;
}

void add_stmt_list(StatementList** start, Stmt* stmt) {
  if (!(*start)) {
    *start = alloc_stmt_list();
    (*start)->value = stmt;
    return;
  } else {
    StatementList* end = *start;
    while (end->next) end = end->next;

    StatementList* new = alloc_stmt_list();
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

void free_stmt_list(StatementList* start) {
  StatementList* curr = start;
  while (curr) {
    free_stmt(curr->value);
    curr = curr->next;
  }
}

/* --------------------------- Symbol table --------------------------- */

Symbol* alloc_symbol(char* name, ExprResult value) {

  // TODO: Add commands for printing tokens, ast, 3 address code, symtab
  Symbol* alloc = malloc(sizeof(Symbol));
  // TODO: ensure_non_null
  alloc->name = strdup(name);
  alloc->value = value;
  alloc->next = NULL;
  return alloc;
}

void add_symbol(SymbolTable** head, char* name, ExprResult value) {
  if (!(*head)) {
    *head = alloc_symbol(name, value);
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

    Symbol* new = alloc_symbol(name, value);
    end->next = new;
  }
}

ExprResult symbol_get(SymbolTable* head, char* name) {
  Symbol* curr = head;

  while (curr) {
    if (strcmp(curr->name, name) == 0) {
      return curr->value;
    }
    curr = curr->next;
  }

  fprintf(stderr, "Runtime error: undefined variable '%s'\n", name);
  exit(1);
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

void free_symtab(SymbolTable* head) {
  Symbol* curr = head;

  while (curr) {
    free(curr->name);
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
  free_stmt_list(parse_result);
  free_symtab(symtab);

  return 0;
}
