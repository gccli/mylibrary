SNORT=/root/tmp/snort/src
INC=-I. -I../.. -I$(SNORT) -I$(SNORT)/sfutil

SNORT_SYNAMIC_PREPROC=$(SNORT)/sfutil/libsfutil.a
CFLAGS= -c -g -O2 -Wall $(INC)
CPPFLAGS=-std=c++0x $(CFLAGS)
LDFLAGS=

BIN=a.out
all:$(BIN)

OBJS=acstd.o test.o

a.out:$(OBJS) $(SNORT_SYNAMIC_PREPROC)
	$(CXX) $^ -o $@ $(LDFLAGS)

%.o:%.cpp
	$(CXX) $(CPPFLAGS)  $^ -o $@

%.o:%.c
	$(CC) $(CFLAGS) $^ -o $@


clean:
	$(RM) $(BIN) $(OBJS) *.o core
