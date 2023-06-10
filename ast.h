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

typedef enum {
  IdentBOp_Plus,
  IdentBOp_Minus,
  IdentBOp_Star,
  IdentBOp_Slash,

  IdentBOp_Gt,
  IdentBOp_Gte,
  IdentBOp_Lt,
  IdentBOp_Lte,
  IdentBOp_EqEq,

  IdentBOp_And,
  IdentBOp_Or,
} IdentBinaryOp;

typedef enum {
  IdentUOp_Minus,
  IdentUOp_Exclamation,
} IdentUnaryOp;

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
  (StringExpr, StrExpr *)
);

datatype(
  IdentExpr,
  (IdentBinaryExpr, char*, IdentBinaryOp, LiteralExpr*),
  (IdentUnaryExpr, IdentUnaryOp, char*),
  (Identifier, char*)
);

datatype(
  Expr,
  (LiteralExpression, LiteralExpr*),
  (IdentExpression, IdentExpr*)
);

typedef Expr Condition;

/* Ensure that an expression evaluates to a boolean */
bool eval_to_condition(Expr* expr);

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
  (DisplayStmt, Expr*),
  (ExprStmt, Expr*),
  (AssignStmt, char*, Expr*),
  (IfStmt, Condition*, TrueStatements*, ElseIfStatement*, ElseStatements*),
  (WhileStmt, Condition*, TrueStatements*)
);

struct StatementList {
  Stmt* value;
  StatementList* next;
  StatementList* prev; // Remove prev ref
};

StatementList* alloc_stmt_list();
void add_stmt_list(StatementList** start, Stmt* stmt);
void eval_stmt_list(StatementList* stmts);
void print_stmt_list(StatementList* ast, int indent);
void ir_stmt_list(StatementList* stmts);
void free_stmt_list(StatementList* stmts);

datatype(
  ExprResult,
  (BooleanResult, bool),
  (NumberResult, double),
  (StringResult, char*)
);

ArithExpr* alloc_aexpr(ArithExpr ast);
double eval_aexpr(ArithExpr* ast);
void print_aexpr(ArithExpr* ast, int indent);
int ir_aexpr(ArithExpr* ast);
void free_aexpr(ArithExpr* ast);

BoolExpr* alloc_bexpr(BoolExpr ast);
bool eval_bexpr(BoolExpr* ast);
void print_bexpr(BoolExpr* ast, int indent);
int ir_bexpr(BoolExpr* ast);
void free_bexpr(BoolExpr* ast);

StrExpr* alloc_sexpr(StrExpr ast);
char* eval_sexpr(StrExpr* ast);
void print_sexpr(StrExpr* ast, int indent);
int ir_sexpr(StrExpr* ast);
void free_sexpr(StrExpr* ast);

LiteralExpr* alloc_literal_expr(LiteralExpr ast);
ExprResult eval_literal_expr(LiteralExpr *);
void print_literal_expr(LiteralExpr* ast, int indent);
int ir_literal_expr(LiteralExpr* ast);
void free_literal_expr(LiteralExpr* ast);

IdentExpr* alloc_ident_expr(IdentExpr ast);
ExprResult eval_ident_expr(IdentExpr *);
void print_ident_expr(IdentExpr* ast, int indent);
int ir_ident_expr(IdentExpr* ast);
void free_ident_expr(IdentExpr* ast);

Expr* alloc_expr(Expr ast);
ExprResult eval_expr(Expr *);
void print_expr(Expr* ast, int indent);
int ir_expr(Expr* ast);
void free_expr(Expr* ast);

Stmt* alloc_stmt(Stmt ast);
void eval_stmt(Stmt* ast);
void print_stmt(Stmt* ast, int indent);
void ir_stmt(Stmt* ast);
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
ExprResult symbol_get(SymbolTable* head, char* name);
void free_symtab(SymbolTable* head);
void print_symtab(SymbolTable* head);
