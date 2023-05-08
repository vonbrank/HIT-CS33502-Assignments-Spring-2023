all: parser executor

executor: executor.c
	gcc executor.c -o executor

parser: main.c lex.yy.c syntax.tab.c node.o
	gcc -lfl main.c syntax.tab.c node.o -o parser 

lex.yy.c: lexical.l syntax.tab.c syntax.tab.h
	flex lexical.l

syntax.tab.c: syntax.y
	bison -d syntax.y

node.o: node.c node.h
	gcc -c node.c

clean:
	rm -f lex.yy.c syntax.tab.c syntax.tab.h node.o parser executor
