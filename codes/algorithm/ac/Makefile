BIN=a.out
INC=-I. -I/usr/local/snort/include/snort

#SNORT_SYNAMIC_PREPROC=/usr/local/snort/lib/snort/dynamic_preproc/libsf_dynamic_preproc.a
SNORT_SYNAMIC_PREPROC=/root/tmp/snort/src/preprocessors/libspp.a\
	/root/tmp/snort/src/sfutil/libsfutil.a
CFLAGS=-c -g -Wall $(INC)
CPPFLAGS=-std=c++0x $(CFLAGS)
LDFLAGS=
all:$(BIN)

OBJS=test.o acsmx.o mpse.o

a.out:$(OBJS) $(SNORT_SYNAMIC_PREPROC)
	$(CXX) $^ -o $@ $(LDFLAGS)

%.o:%.cpp
	$(CXX) $(CPPFLAGS)  $^ -o $@

%.o:%.c
	$(CC) $(CFLAGS) $^ -o $@


clean:
	$(RM) $(BIN) $(OBJS) *.o core
