b: build
build:
	bison -Wcounterexamples -d parser.y
	flex lex.l
	gcc -Iinclude/ -Wall -ftrack-macro-expansion=0 -g -lfl -ly parser.tab.c lex.yy.c ast.c -o parser

r: run
run: build
	./parser
