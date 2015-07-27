include ../../Makefile.common
TARGET=$(LIB)/libdac.a


TEMP=/tmp
OBJFILES=$(TEMP)/libdac.o $(TEMP)/libdacapp.o $(TEMP)/libdacmsg.o

CFLAGS=-g -Wall -I../inc -I$(INC)

all:$(TARGET)
$(TARGET):$(OBJFILES)	
	$(AR) -rc $@ $^


$(TEMP)/%.o:%.cpp
	$(CXX) $(CFLAGS) -c $^ -o $@

clean:
	$(RM) $(TARGET) $(OBJFILES)
