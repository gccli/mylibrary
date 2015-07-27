TARGET=app

CC=g++
CXXFLAGS=-g -Wall
LDFLAGS=

SRCS_APP=$(wildcard *.cpp)
OBJS_APP=$(subst .cpp,.o,$(SRCS_APP))

all:$(TARGET)

app:$(OBJS_APP)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o:%.cpp
	$(CC) $(CXXFLAGS) -c $^ -o $@

clean:
	$(RM) $(OBJS) $(TARGET) *.o 
