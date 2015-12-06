TARGET=producer consumer

CFLAGS=-g -Wall -I.
LDFLAGS=-L. -lrabbitmq


all:$(TARGET)

producer:amqp_producer.c utils.c amqpclient.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
consumer:amqp_consumer.c utils.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
clean:
	$(RM) *.o $(TARGET)
