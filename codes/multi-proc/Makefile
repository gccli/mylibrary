TARGET=libso1.so libso2.so so

CC=g++

SRCS=$(wildcard *.cpp)
OBJS=$(subst .cpp,.o,$(SRCS))

all:$(TARGET)

so:main.o comm.o 
	@make -C p7z
	$(CC) -o $@ $^ -Lp7z -l7z -ldl -lpthread
libso1.so:so1.o comm.o
	$(CC) -shared -Wl,-soname,libso1.so -o $@ $^
libso2.so:so2.o comm.o
	$(CC) -shared -Wl,-soname,libso2.so -o $@ $^

%.o:%.cpp
	$(CC) -g -Wall -fPIC -c $^ -o $@

clean:
	$(RM) $(OBJS) $(TARGET)
	make -C p7z clean