extern int yylineno;
void yyerror(const char *, ...);

struct Ast {
  int nodetype;
  struct Ast *left;
  struct Ast *right;
};

typedef struct Ast Ast ;

typedef struct {
  int nodetype;
  double number;
} NumVal;

Ast *newast(int nodetype, Ast *left, Ast *right);
Ast *newnum(double d);

double eval(Ast *);

void treefree(Ast *);
