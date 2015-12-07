TARGET=producer consumer

CFLAGS=-g -Wall -I. -Irabbitmq-c/librabbitmq -I$(HOME)/mylibrary/utils
LDFLAGS=-L. -lrabbitmq


ALL_SRCS=$(wildcard *.c)
COMMON_SRCS=$(filter-out consumer.c producer.c, $(ALL_SRCS))

all:$(TARGET)

producer:producer.c $(COMMON_SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
consumer:consumer.c $(COMMON_SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
clean:
	$(RM) *.o $(TARGET)
