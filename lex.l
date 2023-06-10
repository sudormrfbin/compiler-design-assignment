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

%x STRING_STATE

%%

"+" |
"-" |
"*" |
"/" |
"!" |
"=" |
"(" |
")"     { return yytext[0]; }

"=="    { return EQEQ;}
">"     { return GT;  }
"<"     { return LT;  }
">="    { return GTE; }
"<="    { return LTE; }
"&&"    { return AND; }
"||"    { return OR;  }

"true"  { return TRUE;  }
"false" { return FALSE; }

"display" { return DISPLAY; }

"if"    { return IF;    }
"then"  { return THEN;  }
"else"  { return ELSE;  }
"endif" { return ENDIF; }

"while"    { return WHILE;    }
"do"       { return DO;       }
"endwhile" { return ENDWHILE; }

\n      { return EOL; }

[ \t]   { } /* ignore whitespace */
"//".+  { } /* ignore comments   */

[0-9]+("."[0-9]+)?  {
    yylval.number = atof(yytext);
    return NUMBER; 
}

\"\" { yylval.string = strdup(""); return STRING; } /* Empty string not being recognized by STRING_STATE, so special case it */

\"                      { BEGIN(STRING_STATE);                           }
<STRING_STATE>[^\"\n]*  { yylval.string = strdup(yytext); return STRING; }
<STRING_STATE>\"        { BEGIN(INITIAL);                                }

[_a-zA-Z][_a-zA-Z0-9]*  { yylval.ident = strdup(yytext); return IDENT; }

.|\n { printf("Unrecognized character: %s", yytext); } /* FIXME: lex.l:65: warning, -s option given but default rule can be matched */
    
%%
