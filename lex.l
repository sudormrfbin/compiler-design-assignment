/*
yywrap() is a function called automatically when flex
finishes reading the input. It controls whether or not to
lex another input file, and since we only read one file,
we disable this functionality using noyywrap.
*/
%option noyywrap nodefault yylineno nounput noinput

%{

#include "parser.tab.h"

%}

%%

"+" |
"-" |
"*" |
"/" |
">" |
"<" |
"="        return yytext[0];

"=="       return EQEQ;
">="       return GTE;
"<="       return LTE;

"display"  return DISPLAY;

"if"       return IF;
"then"     return THEN;
"else"     return ELSE;
"endif"    return ENDIF;

"while"    return WHILE;
"do"       return DO;
"endwhile" return ENDWHILE;

\n+        return EOL;

[ \t]      /* ignore whitespace */
"//".+     /* ignore comments   */

[0-9]+       yylval.number = atoi(yytext); return NUMBER;

[_a-zA-Z]+   yylval.ident = strdup(yytext); return IDENT;

%%
