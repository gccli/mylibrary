include openssl.mk

CFLAGS=-g -Wall -fPIC -I$(OSSLINC)

LDFLAGS=$(OSSL_LDFLAGS)

TARGET=libencrypt.so

OBJS=crypt_common.o cipher.o crypt.o rsakey.o

libencrypt.so:$(OBJS)
	$(CXX) -shared -Wl,-soname,libencrypt.so -o $@ $^ $(LDFLAGS)

%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

%.o:%.cpp
	$(CXX) $(CFLAGS) -c $^ -o $@

crypt:crypt.cpp
	$(CXX) $(CFLAGS) -D_CRYPT_MAIN crypt.cpp -o $@ $(LDFLAGS) -Wl,-rpath=. -L. -lencrypt
