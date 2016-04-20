P7ZIP=p7zip_15.14.1

include $(P7ZIP)/makefile.crc32
include $(P7ZIP)/makefile.machine

TARGET_FLAGS=$(CC_EXE)
LOCAL_LINK=$(LINK_EXE)
LIBS=$(LOCAL_LIBS_DLL)

RM=rm -f

CFLAGS=-c -I. \
-I$(P7ZIP)/C \
-I$(P7ZIP)/CPP/myWindows \
-I$(P7ZIP)/CPP/include_windows \
-I$(P7ZIP)/CPP \
$(ALLFLAGS) $(ALLFLAGS_C)

CXXFLAGS=-c -I. \
-I$(P7ZIP)/C \
-I$(P7ZIP)/CPP/myWindows \
-I$(P7ZIP)/CPP/include_windows \
-I$(P7ZIP)/CPP \
$(ALLFLAGS) $(ALLFLAGS_CPP)


PCH_NAME=$(PRE_COMPILED_HEADER)

include makefile.list

$(PROG): $(OBJS)
	$(CXX) $(ALLFLAGS) $(ALLFLAGS_CPP) $(LOCAL_LINK) -o $(PROG) $(LDFLAGS) $(OBJS) $(LIBS)

debug:
	@echo "LOCAL_FLAGS=$(LOCAL_FLAGS)"
	@echo "ALLFLAGS=$(ALLFLAGS)"

depend:
	rm -f makefile.depend
	$(CXX) -MM $(CXXFLAGS) $(SRCS)   > makefile.depend

clean:
	-@$(RM) $(PROG) core *.exe *.o *~ .*.swp *.orig *.gch $(PROGS)
