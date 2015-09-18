include ../Makefile.common

TARGET=$(LIB)/libemail.a

CC=gcc
CFLAGS=-g -Wall -I$(INC)
OBJS=base64.o imap.o

all:$(TARGET)
%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(LIB)/libemail.a:$(OBJS)
	$(AR) rc $@ $^

clean:
	$(RM) $(TARGET) $(OBJS)
