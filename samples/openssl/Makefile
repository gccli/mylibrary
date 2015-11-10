include ../../Makefile.common

TARGET=client server mailclient crypt

SRCS=$(wildcard *.c)
OBJS=$(subst .c,.o,$(SRCS))

COBJS=client.o infrastucture.o
SOBJS=server.o infrastucture.o
MOBJS=mailclient.o infrastucture.o

CRYPTOBJS=crypt.o cipher.o hexdump.o utilfile.o

CFLAGS=-g -Wall -I$(INC)
LDFLAGS=-lssl -lcrypto -lpthread -lm

all: $(OBJS) $(TARGET)

%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@
client:$(COBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
server:$(SOBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
mailclient:$(MOBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

crypt:$(CRYPTOBJS)
	$(CXX) -o $@ $^ -L$(HOME)/tmp/openssl/lib $(LDFLAGS) -ldl

%.o:%.cpp
	$(CXX) $(CFLAGS) -I$(HOME)/tmp/openssl/include -c $^ -o $@

%.o:$(UTILS)/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	@$(RM) $(OBJS) $(TARGET)
