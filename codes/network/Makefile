include ../../Makefile.common

TARGET=epoll-server client server i6udp

SRCS=$(wildcard *.c)
OBJS=$(subst .c,.o,$(SRCS))

COBJS=ipv6client.o
SOBJS=ipv6server.o
I6OBJ=ipv6udp.o
CFLAGS=-g -Wall -I$(INC)
LDFLAGS=-L$(LIB) -lutilext -lssl -lpthread -lm

all:$(TARGET)

epoll-server:epoll-server.cpp
	$(CXX) -g $^ -o $@

client:$(COBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)
server:$(SOBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)
i6udp:$(I6OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o:%.c
	$(CXX) $(CFLAGS) -c $^ -o $@

clean:
	@$(RM) $(OBJS) $(TARGET)
