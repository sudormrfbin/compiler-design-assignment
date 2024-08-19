# Pseudo Lang Compiler

A compiler made using C, [flex] and [bison] for a language that reads like pseudo code.
Also comes with compiler options to print intermediate stages of the compiler to inspect it's inner workings.

[flex]: https://github.com/westes/flex
[bison]: https://www.gnu.org/software/bison/

## Dependencies

1. [`datatype99`]: Implementation of sum types in pure C to be used to represent and manipulate
the AST concisely.

The header files are duplicated under the `include/` directory.

[`datatype99`]: https://github.com/Hirrolot/datatype99
[`cvector`]: https://github.com/eteran/c-vector/

## Building

Install `make`, and then build the program with `make build`. The compiler binary will be built, called `pseudoc`

## Usage

```bash
$ ./pseudoc -h
Usage: psuedoc [options] filename

    -h, --help    show this help message and exit

Debug options
    -t, --tokens  print token stream
    -a, --ast     print syntax tree
    -s, --symtab  print symbol table
    -i, --ir      print 3 address intermediate code

```

Some test files are provided in the `tests` directory.

## Syntax Showcase

```
name = "John"
add_last_name = true

if add_last_name then
	name = name + " Doe"
else
	display "not adding last name"
endif

display name

for i = 1 to 2 * 2 do
	display i
endfor

max = 5
string = "a"

while max >= 0 do
	string = string + "a"
	display string
	max = max - 1
endwhile
```