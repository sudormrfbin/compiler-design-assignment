b: build
build:
	bison -d parser.y
	flex lex.l
	gcc -lfl -ly -o parser parser.tab.c lex.yy.c ast.c

r: run
run: build
	./parser
