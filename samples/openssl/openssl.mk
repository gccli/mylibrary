OPENSSL=$(HOME)/tmp/openssl
OSSLLIB=$(OPENSSL)/lib
OSSLINC=$(OPENSSL)/include

OSSL_LDFLAGS=-Wl,-rpath=$(OSSLLIB) -L$(OSSLLIB) -lssl -lcrypto -lpthread -lm -ldl
