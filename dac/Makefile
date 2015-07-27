include ../Makefile.common

MYROOT=$(MYLIBRARY)/dac
TARGET=dac
DACLIB=$(LIB)/libdac.a
all:$(TARGET)

TEMP=temp

SRCS =$(wildcard *.cpp)
SRCS+=$(wildcard event/*.cpp)
SRCS+=$(wildcard base/*.cpp)
SRCS+=$(wildcard ip/*.cpp)
SRCS+=$(wildcard net/*.cpp)

INCLUDES  = -I$(MYROOT)
INCLUDES += -I$(MYROOT)/inc
INCLUDES += -I$(MYROOT)/event
INCLUDES += -I$(MYROOT)/ip
INCLUDES += -I$(MYROOT)/net
INCLUDES += -I$(MYROOT)/base

CFLAGS=-g -Wall $(INCLUDES)
LDFLAGS=-L$(LIB) -lutilext -lpthread -lm -lrt

OBJS=$(subst .cpp,.o,$(SRCS))

dac:$(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

app:$(DACLIB)
	@make -C app

$(DACLIB):
	make -C libdac

%.o:%.cpp
	$(CXX) $(CFLAGS) -c $^ -o $@

clean:
	$(RM) -r $(TARGET) temp $(OBJS) *.log core.*
	@make -C libdac clean
	@find . -name "*.o" | xargs rm -f
