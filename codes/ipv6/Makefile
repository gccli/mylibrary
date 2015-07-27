TARGET=ipv6

SRCS=$(wildcard *.c)
OBJS=$(subst .c,.o,$(SRCS))
CFLAGS=-g -Wall -I.
LDFLAGS=-L$(LIB) -lpthread -lm

all:$(TARGET)

ipv6:$(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o:%.c
	$(CXX) $(CFLAGS) -c $^ -o $@

clean:
	@$(RM) $(OBJS) $(TARGET)
