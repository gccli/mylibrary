include ../../Makefile.common
TARGET=mail
all:$(TARGET)

TEMP=/tmp
OBJ=$(TEMP)/main.o

CFLAGS=-g -Wall -I$(INC)
LDFLAGS=-L$(LIB) -lemail -lssl

mail:$(OBJ)	
	$(CC) -o $@ $^ $(LDFLAGS)

$(TEMP)/%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	$(RM) $(TARGET) $(OBJ) core.*
