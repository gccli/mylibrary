TARGET=libso1.so libso2.so so

CC=g++

SRCS=$(wildcard *.cpp)
#OBJS=$(subst .cpp,.o,$(SRCS))
OBJS=main.o so1.o so2.o comm.o

all:$(TARGET)

so:main.o comm.o
	$(CC) -o $@ $^ -L. -lso1 -lso2
libso1.so:so1.o comm.o
	$(CC) -shared -Wl,-soname,libso1.so -o $@ $^
libso2.so:so2.o comm.o
	$(CC) -shared -Wl,-soname,libso2.so -o $@ $^

%.o:%.cpp
	$(CC) -g -Wall -fPIC -c $^ -o $@

clean:
	$(RM) $(OBJS) $(TARGET)
