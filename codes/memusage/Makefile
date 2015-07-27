TARGET=memusage memdump mallhook

CC=g++

SRCS=$(wildcard *.cpp)
OBJS=$(subst .cpp,.o,$(SRCS))

all:$(TARGET)


memusage:memusage.o
	$(CC) -o $@ $^

mallhook:mallhook.o
	$(CC) -o $@ $^

memdump:memdump.o
	$(CC) -o $@ $^ -L../../lib -lutilext -lrt -lpthread

%.o:%.cpp
	$(CC) -g -Wall -I../../inc -c $^ -o $@

clean:
	$(RM) $(OBJS) $(TARGET)
