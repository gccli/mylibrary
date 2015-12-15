all:include

include:include.l
	flex include.l
	gcc -g -Wall lex.yy.c -o $@ -lfl

clean:
	rm -f lex.yy.c include
