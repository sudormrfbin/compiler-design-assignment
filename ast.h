#include "datatype99.h"
#include <stdbool.h>

typedef struct StatementList StatementList;

extern int yylineno;
extern void yyerror(const char *, ...);

typedef enum {
  BinaryOp_Add,
  BinaryOp_Sub,
  BinaryOp_Mul,
  BinaryOp_Div,
} BinaryOp;

typedef enum {
  UnaryOp_Minus,
} UnaryOp;

typedef enum {
  RelationalEqual,
  Greater,
  GreaterOrEqual,
  Less,
  LessOrEqual,
} RelationalOp;

typedef enum {
  And,
  Or,
  LogicalEqual,
} LogicalOp;

datatype(
  ArithExpr,
  (BinaryAExpr, ArithExpr *, BinaryOp, ArithExpr *),
  (UnaryAExpr, UnaryOp, ArithExpr *), // Use NegatedArithExpr and remove UnaryOp
  (Number, double)
);

datatype(
  BoolExpr,
  (RelationalArithExpr, ArithExpr*, RelationalOp, ArithExpr*),
  (LogicalBoolExpr, BoolExpr*, LogicalOp, BoolExpr*),
  (NegatedBoolExpr, BoolExpr*),
  (Boolean, bool)
);

datatype(
  StrExpr,
  (String, char*),
  (StringConcat, StrExpr*, StrExpr*)
);

datatype(
  LiteralExpr,
  (BooleanExpr, BoolExpr *),
  (ArithmeticExpr, ArithExpr *),
  (StringExpr, StrExpr *),
  (IdentExpr, char*)
);

typedef BoolExpr Condition;
typedef StatementList TrueStatements;
typedef StatementList ElseStatements;
typedef struct ElseIfStatement ElseIfStatement;

struct ElseIfStatement {
  Condition* condition;
  TrueStatements* true_stmts;

  ElseIfStatement* next;
};

datatype(
  Stmt,
  (DisplayStmt, LiteralExpr*),
  (ExprStmt, LiteralExpr*),
  (AssignStmt, char*, LiteralExpr*),
  (IfStmt, Condition*, TrueStatements*, ElseIfStatement*, ElseStatements*)
);

struct StatementList {
  Stmt* value;
  StatementList* next;
  StatementList* prev; // Remove prev ref
};

StatementList* alloc_stmt_list();
void add_stmt_list(StatementList** start, Stmt* stmt);
void eval_stmt_list(StatementList* stmts);
void free_stmt_list(StatementList* stmts);
void print_stmt_list(StatementList* ast, int indent);

datatype(
  ExprResult,
  (BooleanResult, bool),
  (NumberResult, double),
  (StringResult, char*)
);

// TODO: Rename to alloc_* and free_*
ArithExpr* alloc_aexpr(ArithExpr ast);
double eval_aexpr(ArithExpr* ast);
void print_aexpr(ArithExpr* ast, int indent);
void free_aexpr(ArithExpr* ast);

BoolExpr* alloc_bexpr(BoolExpr ast);
bool eval_bexpr(BoolExpr* ast);
void print_bexpr(BoolExpr* ast, int indent);
void free_bexpr(BoolExpr* ast);

StrExpr* alloc_sexpr(StrExpr ast);
char* eval_sexpr(StrExpr* ast);
void print_sexpr(StrExpr* ast, int indent);
void free_sexpr(StrExpr* ast);

LiteralExpr* alloc_literal_expr(LiteralExpr ast);
ExprResult eval_literal_expr(LiteralExpr *);
void print_literal_expr(LiteralExpr* ast, int indent);
void free_literal_expr(LiteralExpr* ast);

Stmt* alloc_stmt(Stmt ast);
void eval_stmt(Stmt* ast);
void print_stmt(Stmt* ast, int indent);
void free_stmt(Stmt* ast);

ElseIfStatement* alloc_else_if(Condition* cond, TrueStatements* stmts);
void add_else_if(ElseIfStatement** start, Condition* cond, TrueStatements* stmts);
bool eval_else_if(ElseIfStatement* head);
void print_else_if(ElseIfStatement* ast, int indent);
void free_else_if(ElseIfStatement* head);

typedef struct Symbol Symbol;
typedef Symbol SymbolTable;

struct Symbol {
  char* name;
  ExprResult value;

  Symbol* next;
};

void add_symbol(SymbolTable** head, char* name, ExprResult value);
ExprResult* symbol_get(SymbolTable* head, char* name);
void free_symtab(SymbolTable* head);
void print_symtab(SymbolTable* head);
