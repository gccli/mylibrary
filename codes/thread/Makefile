CC=g++

SRCS=$(wildcard *.cpp)
OBJS=$(subst .cpp,.o,$(SRCS))
TARGET=$(subst .cpp,,$(SRCS))

all:$(TARGET)
%:%.cpp
	$(CC) -g $^ -o $@ -lpthread

clean:
	$(RM) $(OBJS) $(TARGET)
