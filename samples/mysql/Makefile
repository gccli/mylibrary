include ../../Makefile.common

TARGET=mysql sqlite

SRCS=$(filter-out $(wildcard *.c), sqlite.c)
OBJS=$(subst .c,.o,$(SRCS))

CFLAGS=-g -Wall -I$(INC)
LDFLAGS= -L$(LIB) -lutilext -lmysqlclient -lpthread

all: $(TARGET)

mysql:mysql.o
	$(CC) -o $@ $^ $(LDFLAGS)

sqlite:sqlite.o
	$(CC) -o $@ $^ -lsqlite3

%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	$(RM) $(OBJS) *.o $(TARGET)
