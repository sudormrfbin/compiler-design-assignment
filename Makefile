gen:
	bison -Wcounterexamples -d parser.y
	flex lex.l

build: gen
	gcc -Wextra -Wall parser.tab.c lex.yy.c main.c
