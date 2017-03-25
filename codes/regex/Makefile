TARGET=regex pcre
CC=g++
CXXFLAGS=-g -Wall -I../../inc

all:$(TARGET)

regex:regex.o
	$(CC) -o $@ $^
pcre:pcre.c
	cc -g -Wall -o $@ $^ -lpcre

boost_regex_thread:boost_regex_thread.o
	$(CC) -o $@ $^ -lboost_regex -lpthread

%.o:%.cpp
	$(CC) $(CXXFLAGS) -c $^ -o $@

clean:
	$(RM) *.o $(TARGET)
