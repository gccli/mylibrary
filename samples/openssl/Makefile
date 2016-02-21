include ../../Makefile.common
include openssl.mk

TARGET=libencrypt.so client server
#mailclient sslproxy

# common file defined in mylibrary/utils



SRCS=$(wildcard *.c)
OBJS=$(subst .c,.o,$(SRCS))

COMOBJS=infrastucture.o sslcommon.o callback.o hexdump.o


COBJS=client.o $(COMOBJS)
SOBJS=server.o $(COMOBJS)
MOBJS=mailclient.o infrastucture.o

PROXYOBJS=ssl_proxy.o infrastucture.o

CFLAGS=-g -Wall -I$(INC) -I$(OSSLINC)
LDFLAGS=$(OSSL_LDFLAGS)


all: $(OBJS) $(TARGET)

%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

libencrypt.so:
	$(MAKE) -f encrypt.mk

client:$(COBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
server:$(SOBJS)
	$(CC) -o $@ $^ $(LDFLAGS) -Wl,-rpath=. -L. -lencrypt
mailclient:$(MOBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
sslproxy:$(PROXYOBJS)
	$(CC) -o $@ $^ $(LDFLAGS) -Wl,-rpath=. -L. -lencrypt

%.o:%.cpp
	$(CXX) $(CFLAGS) -c $^ -o $@

%.o:$(UTILS)/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	@$(RM) $(OBJS) $(TARGET) *.o core
