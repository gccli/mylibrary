all:rbtree

rbtree-src = $(wildcard *.c)
rbtree-obj = $(subst .c,.o,$(rbtree-src))

rbtree:$(rbtree-obj)
	$(CC) -g $^ -o $@ -lm

.c.o:
	$(CC) -I. -g -Wall -c -o $@ $<

clean:
	@find . -name "*.o" | xargs rm -f
	$(RM) rbtree
