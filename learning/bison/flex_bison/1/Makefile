ex: calc.l calc.y
	bison -d calc.y
	flex calc.l
	gcc -o $@ -g -Wall calc.tab.c lex.yy.c -lfl

clean:
	rm -f ex *.tab.c *.yy.c *.tab.h
