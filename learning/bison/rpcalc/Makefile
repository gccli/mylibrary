all:calc rpcalc mfcalc

calc:calc.y
	bison calc.y
	gcc -g -Wall calc.tab.c -o $@ -lm

rpcalc:rpcalc.y
	bison rpcalc.y
	gcc -g -Wall rpcalc.tab.c -o $@ -lm

mfcalc:mfcalc.y
	bison mfcalc.y
	gcc -g -Wall mfcalc.tab.c -o $@ -lm

clean:
	rm -f calc rpcalc mfcalc
	rm -f *tab.c
