gen:
	bison -Wcounterexamples -d parser.y
	flex lex.l

build: gen
	gcc -Wextra -Wall -g parser.tab.c lex.yy.c ast.c
