TARGET=ss
CFLAGS=
ifeq ($(opt),1)
CFLAGS=-02
else
CFLAGS=-g -O0
endif

sort-src = $(wildcard *.c)
sort-obj = $(subst .c,.o,$(sort-src))

ss:$(sort-obj)
	$(CC) -o $@ $(sort-obj) -lm

clean:
	@find . -name "a.out" | xargs rm -f
	@find . -name "*.o" | xargs rm -f
	$(RM) $(TARGET)
