BIN=a.out acsmx
all:$(BIN)


OBJS=test.o acsmx.o mpse.o

a.out:$(OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

%.o:%.cpp
	$(CXX) -I. -c -g -Wall -std=c++0x $^ -o $@

%.o:%.c
	$(CC) -I. -c -g -Wall $^ -o $@

acsmx:*.c
	gcc -g -Wall -DACSMX_MAIN  $^ -o $@

clean:
	$(RM) $(BIN) $(OBJS) *.o core
