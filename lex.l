/*
yywrap() is a function called automatically when flex
finishes reading the input. It controls whether or not to
lex another input file, and since we only read one file,
we disable this functionality using noyywrap.
*/
%option noyywrap nodefault yylineno nounput noinput

%{

#include "ast.h"
#include "parser.tab.h"

%}

%%

"+" |
"-" |
"*" |
"/" |
"(" |
")"     { return yytext[0];       }
\n      { return EOL;             }
[ \t]   { /* ignore whitespace */ }
"//".+  { /* ignore comments   */ }

[0-9]+("."[0-9]+)?  {
    yylval.number = atof(yytext);
    return NUMBER; 
  }

.       { printf("Mystery character %c\n", *yytext); }

%%
