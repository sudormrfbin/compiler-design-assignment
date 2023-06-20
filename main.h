typedef struct AstNode AstNode;
typedef struct AstNodeList AstNodeList;
typedef struct Symbol Symbol;
typedef enum AstNodeKind AstNodeKind;

void yyerror(const char *s);

// Symbol Table

struct Symbol {
  char *name;
  int value;
};
void add_symbol(char *name, int value);
int lookup_symbol(char *name);

// Simple linked list for storing a list of statements.
struct AstNodeList {
  AstNode* node;
  AstNodeList* next;
};
AstNodeList* mallocNodeList(AstNode* node);
void addNodeList(AstNodeList* begin, AstNode* newNode);

// AstNode

enum AstNodeKind {
  nkNumber,
  nkIdent,
  nkExpr,
  nkIf,
  nkWhile,
  nkDisplay,
  nkAssign,
};
// Specifies all possible parse tree nodes
struct AstNode {
  AstNodeKind kind;

  union {
    int number;
    char *ident;

    struct {
      AstNode *left;
      char op;
      AstNode *right;
    } expr;

    struct {
      AstNode *condition;
      AstNodeList *true_stmts;
      AstNodeList *else_stmts;
    } ifStmt;

    struct {
      AstNode *condition;
      AstNodeList *true_stmts;
    } whileStmt;

    struct {
      AstNode *expr;
    } display;

    struct {
      AstNode *rhs;
      char *ident;
    } assign;
  } data;
};
AstNode *newNode(AstNodeKind kind);
AstNode *newNumber(int value);
AstNode *newIdent(char* ident);
AstNode *newExpr(AstNode* left, char op, AstNode* right);
AstNode *newIf(AstNode* condition, AstNodeList* true_stmts, AstNodeList* else_stmts);
AstNode *newWhile(AstNode* condition, AstNodeList* true_stmts);
AstNode *newDisplay(AstNode* expr);
AstNode *newAssign(char* ident, AstNode* expr);

int evalExpr(AstNode* expr);
void evalIfStmt(AstNode* node);
void evalWhileStmt(AstNode* node);
void evalDisplay(AstNode* node);
void evalAssign(AstNode* node);
void evalAstNode(AstNode* node);
void evalNodeList(AstNodeList* list);

extern int IR_TEMP_IDX;

extern int IR_LABEL_IDX;
