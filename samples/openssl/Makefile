include ../../Makefile.common

OPENSSL=$(HOME)/tmp/openssl
OSSLLIB=$(OPENSSL)/lib
OSSLINC=$(OPENSSL)/include


TARGET=client server mailclient crypt

# common file defined in mylibrary/utils
COMMOBJ=hexdump.o utilfile.o


SRCS=$(wildcard *.c)
OBJS=$(subst .c,.o,$(SRCS))

COBJS=client.o infrastucture.o
SOBJS=server.o infrastucture.o
MOBJS=mailclient.o infrastucture.o

CRYPTOBJS=$(COMMOBJ) crypt_common.o cipher.o crypt.o rsakey.o

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
	$(CXX) $(CFLAGS) -D_CRYPT_MAIN -c $^ -o $@

%.o:$(UTILS)/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	@$(RM) $(OBJS) $(TARGET) *.o core
