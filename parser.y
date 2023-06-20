%{

int yylex();

%}

%union {
  int number;
  char* ident;
}

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

%left EQEQ
%left '-' '+'
%left '*' '/'

%%

program: stmt_list

stmt_list: stmt
  | stmt_list stmt

stmt: display_stmt
  | if_stmt
  | while_stmt
  | assign_stmt

assign_stmt: IDENT '=' expr EOL

display_stmt: DISPLAY expr EOL

if_stmt: IF expr THEN EOL stmt_list ENDIF EOL
  | IF expr THEN EOL stmt_list ELSE EOL stmt_list ENDIF EOL

while_stmt: WHILE expr DO EOL stmt_list ENDWHILE EOL

expr: expr '+' expr
  | expr '-' expr
  | expr '*' expr
  | expr '/' expr
  | NUMBER             
  | IDENT
