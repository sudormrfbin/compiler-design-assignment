/*
yywrap() is a function called automatically when flex
finishes reading the input. It controls whether or not to
lex another input file, and since we only read one file,
we disable this functionality using noyywrap.
*/
%option noyywrap

%{

#include "parser.tab.h"

%}

%%

"+"     { return ADD; }
"-"     { return SUB; }
"*"     { return MUL; }
"/"     { return DIV; }
"|"     { return ABS; }
[0-9]+  { yylval = atoi(yytext); return NUMBER; }
\n      { return EOL; }
[ \t]   { /* ignore whitespace */ }
.       { printf("Mystery character %c\n", *yytext); }

%%
