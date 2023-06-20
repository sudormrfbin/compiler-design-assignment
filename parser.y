%{
#include "main.h"
#include <stddef.h>

int yylex();
%}

%union {
  struct AstNode* node;
  struct AstNodeList* list;
  int number;
  char* ident;
}

%type <node> expr while_statement if_statement display_statement assign_statement statement
%type <list> statement_list program

%token <number> NUMBER
%token <ident>  IDENT

%token EOL
%token GT GTE LT LTE
%token DISPLAY
%token IF THEN ELSE ENDIF
%token WHILE DO ENDWHILE

/* Explicitly declare precedence instead of implicitly doing it
   through the grammar.
*/

%left EQEQ '>' '<' GTE LTE
%left '-' '+'
%left '*' '/'

%%

program:
  statement_list { evalNodeList($statement_list); }

statement_list:
  statement { $$ = mallocNodeList($statement); }
  | statement_list statement { addNodeList($1, $statement); }

statement:
  display_statement
  | if_statement
  | while_statement
  | assign_statement

assign_statement:
  IDENT '=' expr EOL { $$ = newAssign($1, $expr); }

display_statement:
  DISPLAY expr EOL { $$ = newDisplay($expr); }

if_statement:
  IF expr THEN EOL statement_list ENDIF EOL {
    $$ = newIf($expr, $statement_list, NULL);
  }
  | IF expr THEN EOL statement_list ELSE EOL statement_list ENDIF EOL {
    $$ = newIf($expr, $5, $8);
  }

while_statement:
  WHILE expr DO EOL statement_list ENDWHILE EOL {
    $$ = newWhile($expr, $statement_list);
  }

expr:
  expr '+' expr     { $$ = newExpr($1, '+', $3); }
  | expr '-' expr   { $$ = newExpr($1, '-', $3); }
  | expr '*' expr   { $$ = newExpr($1, '*', $3); }
  | expr '/' expr   { $$ = newExpr($1, '/', $3); }
  | expr '>' expr   { $$ = newExpr($1, '>', $3); }
  | expr '<' expr   { $$ = newExpr($1, '<', $3); }
  | expr GTE expr   { $$ = newExpr($1, 'G', $3); }
  | expr LTE expr   { $$ = newExpr($1, 'L', $3); }
  | expr EQEQ expr  { $$ = newExpr($1, 'E', $3); }
  | NUMBER          { $$ = newNumber($1);        }
  | IDENT           { $$ = newIdent($1);         }
