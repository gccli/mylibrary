SNORT=/root/tmp/snort/src
INC=-I. -I../.. -I$(SNORT)/dynamic-preprocessors/include

SNORT_SYNAMIC_PREPROC=$(SNORT)/preprocessors/libspp.a\
	$(SNORT)/sfutil/libsfutil.a
CFLAGS=-c -g -Wall $(INC)
CPPFLAGS=-std=c++0x $(CFLAGS)
LDFLAGS=

BIN=a.out
all:$(BIN)

OBJS=test.o mpse.o

a.out:$(OBJS) $(SNORT_SYNAMIC_PREPROC)
	$(CXX) $^ -o $@ $(LDFLAGS)

%.o:%.cpp
	$(CXX) $(CPPFLAGS)  $^ -o $@

%.o:%.c
	$(CC) $(CFLAGS) $^ -o $@


clean:
	$(RM) $(BIN) $(OBJS) *.o core
