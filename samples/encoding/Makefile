include ../../Makefile.common
TARGET=base64
all:$(TARGET)

TEMP=/tmp
OBJ=$(TEMP)/encoding.o $(TEMP)/base64.o

CFLAGS=-g -Wall -I$(INC)
LDFLAGS=-L$(LIB)

base64:$(OBJ)	
	$(CC) -o $@ $^ $(LDFLAGS)

$(TEMP)/%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(TEMP)/%.o:$(PROTOCOL)/%.c
	$(CC) $(CFLAGS) -c $^ -o $@
clean:
	$(RM) $(TARGET) $(OBJ) core.*
