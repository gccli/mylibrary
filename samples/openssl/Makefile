include ../../Makefile.common

OPENSSL=$(HOME)/tmp/openssl
OSSLLIB=$(OPENSSL)/lib
OSSLINC=$(OPENSSL)/include


TARGET=client server mailclient crypt

# common file defined in mylibrary/utils
COMMOBJ=hexdump.o utilfile.o crypt_common.o


SRCS=$(wildcard *.c)
OBJS=$(subst .c,.o,$(SRCS))

COBJS=client.o infrastucture.o
SOBJS=server.o infrastucture.o
MOBJS=mailclient.o infrastucture.o

CRYPTOBJS=$(COMMOBJ) cipher.o crypt.o rsa.o
#RSAOBJ=$(COMMOBJ) cipher.o

CFLAGS=-g -Wall -I$(INC) -I$(OSSLINC)
LDFLAGS=-L$(OSSLLIB) -lssl -lcrypto -lpthread -lm -ldl

all: $(OBJS) $(TARGET)

%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

client:$(COBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
server:$(SOBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
mailclient:$(MOBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
#rsa:$(RSAOBJ)
#	$(CXX) -o $@ $^ $(LDFLAGS)
crypt:$(CRYPTOBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)
%.o:%.cpp
	$(CXX) $(CFLAGS) -c $^ -o $@

%.o:$(UTILS)/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	@$(RM) $(OBJS) $(TARGET) *.o core
