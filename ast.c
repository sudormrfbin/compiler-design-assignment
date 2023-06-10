#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "datatype99.h"
#include "parser.tab.h"
#include "argparse.h"

extern SymbolTable* symtab;
extern FILE* yyin;
extern StatementList* parse_result;

int IR_TEMP_IDX = 0;
int IR_LABEL_IDX = 0;

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
      iprintf(ind, "BinaryExpression\n");
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
    }
    of(UnaryAExpr, op, right) {
      iprintf(ind, "UnaryExpression\n");
      ind++;

      iprintf(ind, "Op(");
      switch (*op) {
        case UnaryOp_Minus: printf("-"); break;
      }
      printf(")\n");

      print_aexpr(*right, ind);
      ind--;
    }
    of(Number, num) iprintf(ind, "Number(%g)\n", *num); 
  }

  ind--;
}

int assign_temp_ir(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  printf("t%d = ", IR_TEMP_IDX);
  vprintf(fmt, ap);
  printf("\n");
  return IR_TEMP_IDX++;
}


int ir_aexpr(ArithExpr* ast) {
  match(*ast) {
    of(BinaryAExpr, left, op, right) {
      int l = ir_aexpr(*left);
      int r = ir_aexpr(*right);
      switch (*op) {
        case BinaryOp_Add: return assign_temp_ir("t%d + t%d", l, r);
        case BinaryOp_Sub: return assign_temp_ir("t%d - t%d", l, r);
        case BinaryOp_Mul: return assign_temp_ir("t%d * t%d", l, r);
        case BinaryOp_Div: return assign_temp_ir("t%d / t%d", l, r);
      }
    }
    of(UnaryAExpr, op, right) {
      switch (*op) {
        case UnaryOp_Minus: return assign_temp_ir("- t%d", ir_aexpr(*right));
      }
    }
    of(Number, num) {
      return assign_temp_ir("%g", *num);
    }; 
  }
  unreachable("ir_aexpr");
  return -1;
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
      iprintf(ind, "RelationalExpression\n");
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
      iprintf(ind, "LogicalExpression\n");
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

int ir_bexpr(BoolExpr* ast) {
  match (*ast) {
    of(RelationalArithExpr, left, relop, right) {
      int l = ir_aexpr(*left);
      int r = ir_aexpr(*right);
      switch (*relop) {
        case RelationalEqual: return assign_temp_ir("t%d == t%d", l, r);
        case Greater:         return assign_temp_ir("t%d > t%d", l, r);
        case GreaterOrEqual:  return assign_temp_ir("t%d >= t%d", l, r);
        case Less:            return assign_temp_ir("t%d < t%d", l, r);
        case LessOrEqual:     return assign_temp_ir("t%d <= t%d", l, r);
      }
    }
    of(LogicalBoolExpr, left, logicalop, right) {
      int l = ir_bexpr(*left);
      int r = ir_bexpr(*right);
      switch (*logicalop) {
        case And: return assign_temp_ir("t%d && t%d", l, r);
        case Or: return assign_temp_ir("t%d || t%d", l, r);
        case LogicalEqual: return assign_temp_ir("t%d == t%d", l, r);
      }
    }
    of(NegatedBoolExpr, bexpr) return assign_temp_ir("! t%d", ir_bexpr(*bexpr));
    of(Boolean, boolean) return assign_temp_ir("%s", *boolean ? "true" : "false");
  }

  unreachable("ir_bexpr");
  return -1;
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
      iprintf(ind, "StringConcat\n");
      print_sexpr(*first, ind + 1);
      print_sexpr(*second, ind + 1);
    }
    of(String, str) iprintf(ind, "String(\"%s\")\n", *str);
  }
}

int ir_sexpr(StrExpr* ast) {
  match (*ast) {
    of(StringConcat, first, second) {
      int left = ir_sexpr(*first);
      int right = ir_sexpr(*second);
      return assign_temp_ir("t%d + t%d", left, right);
    }
    of(String, str) return assign_temp_ir("\"%s\"", *str);
  }

  unreachable("ir_sexpr");
  return -1;
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
      // iprintf(ind, "BooleanExpression\n");
      print_bexpr(*bexpr, ind);
    }
    of(ArithmeticExpr, aexpr) {
      // iprintf(ind, "ArithmeticExpression\n");
      print_aexpr(*aexpr, ind);
    }
    of(StringExpr, sexpr) {
      // iprintf(ind, "StringExpression\n");
      print_sexpr(*sexpr, ind);
    }
  }
}

int ir_literal_expr(LiteralExpr* expr) {
  match (*expr) {
    of(BooleanExpr, bexpr) return ir_bexpr(*bexpr);
    of(ArithmeticExpr, aexpr) return ir_aexpr(*aexpr);
    of(StringExpr, sexpr) return ir_sexpr(*sexpr);
  }

  unreachable("ir_literal_expr");
  return -1;
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

int ir_ident_expr(IdentExpr* expr) {
  match (*expr) {
    of(IdentBinaryExpr, ident, op, expr) {
      int r = ir_literal_expr(*expr);
      switch (*op) {
        case IdentBOp_Plus:  return assign_temp_ir("%s + t%d", *ident, r);
        case IdentBOp_Minus: return assign_temp_ir("%s - t%d", *ident, r);
        case IdentBOp_Star:  return assign_temp_ir("%s * t%d", *ident, r);
        case IdentBOp_Slash: return assign_temp_ir("%s / t%d", *ident, r);
        case IdentBOp_Gt:    return assign_temp_ir("%s > t%d", *ident, r);
        case IdentBOp_Gte:   return assign_temp_ir("%s >= t%d", *ident, r);
        case IdentBOp_Lt:    return assign_temp_ir("%s < t%d", *ident, r);
        case IdentBOp_Lte:   return assign_temp_ir("%s <= t%d", *ident, r);
        case IdentBOp_EqEq:  return assign_temp_ir("%s == t%d", *ident, r);
        case IdentBOp_And:   return assign_temp_ir("%s && t%d", *ident, r);
        case IdentBOp_Or:    return assign_temp_ir("%s || t%d", *ident, r);
      }
    }
    of(IdentUnaryExpr, op, ident) {
      switch (*op) {
        case IdentUOp_Minus: return assign_temp_ir("- %s", *ident);
        case IdentUOp_Exclamation: return assign_temp_ir("! %s", *ident);
      }
    }
    of(Identifier, ident) return assign_temp_ir("%s", *ident);
  }

  unreachable("ir_ident_expr");
  return -1;
}

void print_ident_expr(IdentExpr* ast, int ind) {
  match (*ast) {
    of(IdentBinaryExpr, ident, op, expr) {
      iprintf(ind, "BinaryExpression\n");
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
    }
    of(IdentUnaryExpr, op, ident) {
      iprintf(ind, "UnaryExpression\n");
      ind++;

      iprintf(ind, "Op(");
      switch (*op) {
        case IdentUOp_Minus: printf("-"); break;
        case IdentUOp_Exclamation: printf("!"); break;
      }
      printf(")\n");

      iprintf(ind, "Variable(\"%s\")\n", *ident);
      ind--;
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

int ir_expr(Expr* expr) {
  match (*expr) {
    of(LiteralExpression, lexpr) return ir_literal_expr(*lexpr);
    of(IdentExpression, iexpr) return ir_ident_expr(*iexpr);
  }

  unreachable("ir_expr");
  return -1;
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
    of(ForStmt, ident, from, to, stmts) {
      ExprResult from_expr = eval_expr(*from);
      ExprResult to_expr = eval_expr(*to);

      if (!MATCHES(from_expr, NumberResult)) {
        runtime_error("start variable should be a number in for loop");
      }
      if (!MATCHES(to_expr, NumberResult)) {
        runtime_error("for loop end should be a number");
      }

      ifLet(from_expr, NumberResult, from_num) {
        ifLet(to_expr, NumberResult, to_num) {
          for (int i = *from_num; i <= *to_num; i++) {
            add_symbol(&symtab, *ident, NumberResult(i));
            eval_stmt_list(*stmts);
          }
        }
      }
    }
  }
}

void print_stmt(Stmt *ast, int ind) {
  match (*ast) {
    of(DisplayStmt, expr) {
      iprintf(ind, "DisplayStatement\n");
      print_expr(*expr, ind + 1);
    }
    of(ExprStmt, expr) {
      iprintf(ind, "ExpressionStatement\n");
      print_expr(*expr, ind + 1);
    }; 
    of(AssignStmt, ident, value) {
      iprintf(ind, "AssignmentStatement\n");
      iprintf(ind + 1, "Variable(\"%s\")\n", *ident);
      print_expr(*value, ind + 1);
    }; 
    of(IfStmt, condition, true_stmts, else_if, else_stmts) {
      iprintf(ind, "IfStatement\n");

      iprintf(ind + 1, "Condition\n");
      print_expr(*condition, ind + 2);

      iprintf(ind + 1, "TrueStatements\n");
      print_stmt_list(*true_stmts, ind + 2);

      if (*else_if) {
        print_else_if(*else_if, ind + 1);
      }

      if (*else_stmts) {
        iprintf(ind + 1, "ElseStatements\n");
        print_stmt_list(*else_stmts, ind + 2);
      }
    }
    of(WhileStmt, condition, true_stmts) {
      iprintf(ind, "WhileStatement\n");

      iprintf(ind + 1, "Condition\n");
      print_expr(*condition, ind + 2);

      iprintf(ind + 1, "TrueStatements\n");
      print_stmt_list(*true_stmts, ind + 2);
    }
    of(ForStmt, ident, from, to, stmts) {
      iprintf(ind, "ForStatement\n");
      iprintf(ind + 1, "Variable(\"%s\")\n", *ident);

      iprintf(ind + 1, "From\n");
      print_expr(*from, ind + 2);

      iprintf(ind + 1, "To\n");
      print_expr(*to, ind + 2);

      iprintf(ind + 1, "LoopStatements\n");
      print_stmt_list(*stmts, ind + 2);
    }
  }
}

void ir_stmt(Stmt* stmt) {
  match (*stmt) {
    of(DisplayStmt, expr) printf("display t%d\n", ir_expr(*expr));
    of(ExprStmt, expr) ir_expr(*expr); 
    of(AssignStmt, ident, value) printf("%s = t%d\n", *ident, ir_expr(*value));
    of(IfStmt, condition, true_stmts, else_if, else_stmts) {
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

      // TODO: Handle else if
      int true_label = IR_LABEL_IDX++;
      printf("if t%d == true goto L%d\n", ir_expr(*condition), true_label);

      if (*else_stmts) {
        ir_stmt_list(*else_stmts);
      }

      int done_label = IR_LABEL_IDX++;
      printf("goto L%d\n", done_label);

      printf("L%d:\n", true_label);
      ir_stmt_list(*true_stmts);
      printf("L%d:\n", done_label);
    }
    of(WhileStmt, condition, true_stmts) {
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
      int begin_label = IR_LABEL_IDX++;
      printf("L%d:\n", begin_label);

      int true_label = IR_LABEL_IDX++;
      printf("if t%d == true goto L%d\n", ir_expr(*condition), true_label);

      int done_label = IR_LABEL_IDX++;
      printf("goto L%d\n", done_label);

      printf("L%d:\n", true_label);
      ir_stmt_list(*true_stmts);
      printf("goto L%d\n", begin_label);

      printf("L%d:\n", done_label);
    }
    of(ForStmt, ident, from, to, stmts) {
      // Consider a for statement like so:
      // ```
      // for i = 1 to 10 do
      //   stmts
      // endfor
      // rest_of_program
      // ```
      // 
      // Then the corresponding 3 address code will be:
      //
      // ```
      // LBEGIN:
      // i = 1
      // t0 = 10
      // if i <= t0 goto LTRUE
      // goto LDONE
      // LTRUE:
      // stmts
      // i = i + 1
      // goto LBEGIN
      // LDONE:
      // rest_of_program
      // ```
      int begin_label = IR_LABEL_IDX++;
      printf("L%d:\n", begin_label);

      printf("%s = t%d", *ident, ir_expr(*from));
      int true_label = IR_LABEL_IDX++;
      printf("if %s <= t%d goto L%d\n", *ident, ir_expr(*to), true_label);

      int done_label = IR_LABEL_IDX++;
      printf("goto L%d\n", done_label);

      printf("L%d:\n", true_label);
      ir_stmt_list(*stmts);
      printf("%s = %s + 1", *ident, *ident);
      printf("goto L%d\n", begin_label);

      printf("L%d:\n", done_label);
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
    of(ForStmt, ident, from, to, stmts) {
      free(*ident);
      free_expr(*from);
      free_expr(*to);
      free_stmt_list(*stmts);
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
    iprintf(ind, "ElseIfStatement\n");

    iprintf(ind + 1, "Condition\n");
    print_expr(stmt->condition, ind + 2);

    iprintf(ind + 1, "TrueStatements\n");
    print_stmt_list(stmt->true_stmts, ind + 2);

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

void ir_stmt_list(StatementList* start) {
  StatementList* curr = start;
  while (curr) {
    ir_stmt(curr->value);
    curr = curr->next;
  }
}

void print_stmt_list(StatementList* start, int ind) {
  StatementList* curr = start;
  while (curr) {
    // iprintf(ind, "Statement\n");
    print_stmt(curr->value, ind);

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

int main(int argc, const char **argv) {
  static const char *const usages[] = {
      "psuedoc [options] filename",
      NULL,
  };

  int tokens = false;
  int ir = false;
  int ast = false;
  int show_symtab = false;

  // TODO: Add command for 3 address code
  struct argparse_option options[] = {
    OPT_HELP(),
    OPT_GROUP("Debug options"),
    OPT_BOOLEAN('t', "tokens", &tokens, "print token stream", NULL, 0, 0),
    OPT_BOOLEAN('a', "ast", &ast, "print syntax tree", NULL, 0, 0),
    OPT_BOOLEAN('s', "symtab", &show_symtab, "print symbol table", NULL, 0, 0),
    OPT_BOOLEAN('i', "ir", &ir, "print 3 address intermediate code", NULL, 0, 0),
    OPT_END(),
  };

  struct argparse argparse;
  argparse_init(&argparse, options, usages, 0);
  // argparse_describe(&argparse, "\nA brief description of what the program does and how it works.", "\nAdditional description of the program after the description of the arguments.");
  argc = argparse_parse(&argparse, argc, argv);

  if (argc == 0) {
    fprintf(stderr, "filename is required\n");
    exit(1);
  } else {
    if (!(yyin = fopen(*argv, "r"))) {
      perror("could not open file");
      return 1;
    }
  }

  if (tokens != 0) {
    printf("Token printing not yet implemented\n");
    exit(1);
  }

  if (ast != 0) {
    yyparse();
    print_stmt_list(parse_result, 0);
    free_stmt_list(parse_result);
  }

  if (ir != 0) {
    yyparse();
    ir_stmt_list(parse_result);
    free_stmt_list(parse_result);
  }

  if (show_symtab != 0) {
    yyparse();
    eval_stmt_list(parse_result);
    print_symtab(symtab);
    free_stmt_list(parse_result);
    free_symtab(symtab);
  }

  if (!(tokens || ast || show_symtab || ir)) {
    yyparse();
    eval_stmt_list(parse_result);
    free_stmt_list(parse_result);
    free_symtab(symtab);
  }

  return 0;
}
