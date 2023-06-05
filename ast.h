extern int yylineno;
void yyerror(const char *, ...);

typedef struct Ast Ast ;

typedef enum {
  AstType_BinaryOp,
  AstType_Number,
} AstType;

typedef enum {
  BinaryOp_Add,
  BinaryOp_Sub,
  BinaryOp_Mul,
  BinaryOp_Div,
} BinaryOp;

typedef struct {
  Ast* left;
  BinaryOp op;
  Ast* right;
} AstNodeBinary;

struct Ast {
  AstType type;
  union {
    AstNodeBinary binary;
    double number;
  } as;
};

Ast* ast_new();
Ast* ast_new_binary(Ast* left, BinaryOp op, Ast* right);
Ast* ast_new_number(double number);

double eval(Ast *);

void ast_free(Ast *);
