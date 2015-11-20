include ../../Makefile.common
include openssl.mk

TARGET=libencrypt.so client server mailclient sslproxy

# common file defined in mylibrary/utils
COMMOBJ=hexdump.o utilfile.o


SRCS=$(wildcard *.c)
OBJS=$(subst .c,.o,$(SRCS))

COBJS=client.o infrastucture.o
SOBJS=server.o infrastucture.o $(COMMOBJ)
MOBJS=mailclient.o infrastucture.o

PROXYOBJS=ssl_proxy.o infrastucture.o

CFLAGS=-g -Wall -I$(INC) -I$(OSSLINC)
LDFLAGS=-L$(OSSLLIB) -lssl -lcrypto -lpthread -lm -ldl

all: $(OBJS) $(TARGET)

%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

libencrypt.so:
	$(MAKE) -f encrypt.mk

client:$(COBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
server:$(SOBJS)
	$(CC) -o $@ $^ $(LDFLAGS) -L. -lencrypt
mailclient:$(MOBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
sslproxy:$(PROXYOBJS)
	$(CC) -o $@ $^ $(LDFLAGS) -L. -lencrypt
%.o:%.cpp
	$(CXX) $(CFLAGS) -c $^ -o $@

%.o:$(UTILS)/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	@$(RM) $(OBJS) $(TARGET) *.o core
