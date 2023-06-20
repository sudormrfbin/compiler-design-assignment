# Pseudo Lang Compiler

To compile the compiler:

```
gcc parser.tab.c lex.yy.c main.c
```

Run with:

```
./a.exe filename
```

Sample test file is included as `input.test`.

## Regenerate parser and lexer

To re-generate the parser and lexer (only required if `parser.tab.c` and
`lex.yy.c` are deleted or if `parser.y` and `lex.l` are modified):

```
bison -d parser.y
flex lex.l
```
