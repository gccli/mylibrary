TARGET=demo1 demo2 demo3 demo4

all:$(TARGET)
demo1:demo1.c
	$(CC) -g $^ -o $@ 
demo2:demo2.c
	$(CC) -g $^ -o $@ 
demo3:demo3.c
	$(CC) -g $^ -o $@ 
demo4:demo4.c
	$(CC) -g $^ -o $@ 
clean:
	@$(RM) -f  $(TARGET)