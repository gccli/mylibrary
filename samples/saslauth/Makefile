CC=gcc
client_OBJECTS = client.o common.o
server_OBJECTS = server.o common.o

CFLAGS=-g -Wall -I.. -I/usr/include/sasl
LDFLAGS=-lsasl2

all: client server
client:$(client_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

server:$(server_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	$(RM) client server *.o
