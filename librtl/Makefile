include ../Makefile.common


TARGET=$(LIB)/librtl.a

all:$(TARGET)

CXXFLAGS=-g -Wall -I$(INC)
LDFLAGS=-L$(LIB) -lutilext -lpthread

OBJS=rtlrtt.o rtlayer.o rtltimer.o rtlsession.o

$(TARGET):$(OBJS)
	$(AR) -rc $@ $^


%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@ 

clean:
	$(RM) *.o $(TARGET)
