include ../../Makefile.common
TARGET=client server
all:$(TARGET)


TEMP=/tmp
COBJ=$(TEMP)/client.o $(TEMP)/sndrcv.o
SOBJ=$(TEMP)/server.o $(TEMP)/sndrcv.o

CFLAGS=-g -Wall -I$(INC)
LDFLAGS=-L$(LIB) -lutilext -lpthread

client:$(COBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

server:$(SOBJ)	
	$(CXX) -o $@ $^ $(LDFLAGS)

$(TEMP)/%.o:%.cpp
	$(CXX) $(CFLAGS) -c $^ -o $@

clean:
	$(RM) $(TARGET) $(COBJ) $(SOBJ) core.*
