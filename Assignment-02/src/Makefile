all: executor main

executor: executor.c
	gcc executor.c -o executor

main: main.c lex.yy.c syntax.tab.c node.o parser.o string.o vector.o hashmap.o
	gcc -lfl main.c syntax.tab.c node.o parser.o ./utils/string.o ./utils/vector.o ./utils/hashmap.o -o main 

parser: parser.c parser.h lex.yy.c
	gcc -c parser.c

lex.yy.c: lexical.l syntax.tab.c syntax.tab.h
	flex lexical.l

syntax.tab.c: syntax.y
	bison -d syntax.y

node.o: node.c node.h
	gcc -c node.c

string.o: ./utils/string.c ./utils/string.h
	gcc -c ./utils/string.c -o ./utils/string.o

vector.o: ./utils/vector.c ./utils/vector.h
	gcc -c ./utils/vector.c -o ./utils/vector.o

hashmap.o: ./utils/hashmap.c ./utils/hashmap.h
	gcc -c ./utils/hashmap.c -o ./utils/hashmap.o

clean:
	rm -f lex.yy.c syntax.tab.c syntax.tab.h node.o parser.o executor main;
	cd utils && make clean;
