include ../../Makefile.common

TARGET=client server mailclient

SRCS=$(wildcard *.c)
OBJS=$(subst .c,.o,$(SRCS))

COBJS=client.o infrastucture.o
SOBJS=server.o infrastucture.o
MOBJS=mailclient.o infrastucture.o

CFLAGS=-g -Wall -I$(INC)
LDFLAGS=-lssl -lpthread -lm

all: $(OBJS) $(TARGET)

%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@
client:$(COBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
server:$(SOBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
mailclient:$(MOBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	@$(RM) $(OBJS) $(TARGET)
