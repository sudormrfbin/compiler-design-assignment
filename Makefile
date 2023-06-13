b: build
build:
	bison -Wcounterexamples -d parser.y
	flex lex.l
	gcc -Iinclude/ -Wextra -Wall -ftrack-macro-expansion=0 -g argparse.c parser.tab.c lex.yy.c ast.c -o pseudoc

r: run
run: build
	./parser

grammar: parser.y
	sed -n '/%%/,$$p' parser.y | tail -n +3 | sed ':a; /{[^}]*}$$/!{N; ba}; s/{[^}]*}//g; s/\[[^]]*\]//g' > grammar.ebnf
