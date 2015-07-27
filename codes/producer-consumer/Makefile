TARGET=exe1 exe2 exe3 exe4 exe5 exe6 exe7

CFLAGS=-g -Wall

all: $(TARGET)

exe1:produce1.c
	gcc $(CFLAGS) $^ -o $@ -lpthread

exe2:produce2.c
	gcc $(CFLAGS) $^ -o $@ -lpthread

exe3:produce3.c
	gcc $(CFLAGS) $^ -o $@ -lpthread

exe4:produce4.c
	gcc $(CFLAGS) $^ -o $@ -lpthread

exe5:produce5.c
	gcc $(CFLAGS) $^ -o $@ -lpthread

exe6:produce6.c
	gcc $(CFLAGS) $^ -o $@ -lpthread

exe7:produce7.c
	gcc $(CFLAGS) $^ -o $@ -lpthread

exe8:produce8.c
	gcc $(CFLAGS) $^ -o $@ -lpthread

exe9:produce9.c
	gcc $(CFLAGS) $^ -o $@ -lpthread

clean:
	@$(RM) $(TARGET)