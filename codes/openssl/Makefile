TARGET=bignum prime

all:$(TARGET)

bignum:bignum.c
	gcc -g -Wall -o $@ $^ -lssl -lcrypto -lpthread

prime:prime.c
	gcc -g -Wall -o $@ $^ -lssl -lcrypto

clean:
	$(RM) $(TARGET)
