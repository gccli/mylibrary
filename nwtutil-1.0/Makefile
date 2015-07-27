include ../Makefile.common

LIBS= -L$(LIB) -lutilext -lpthread -lrt

CXXFLAGS=-g -Wall -I$(INC)
LDFLAGS=$(LIBS)

OBJS=nwt_config.o\
nwt_nic.o\
nwt_common.o\
nwt_socket.o\
rawsocket.o\
unixsocket.o\
tcpserver.o\
tcpclient.o\
udpserver.o\
udpclient.o\
epollsvr.o\
threadpool.o\
main_aux.o\
main.o

TARGET=nwtutil

all:$(TARGET)
$(TARGET):$(OBJS)
	@echo
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

%.o:%.cpp
	$(CXX) -c $(CXXFLAGS) $^ -o $@

install:
	$(INSTALL) -d $(DESTDIR)$(bindir) $(DESTDIR)$(sbindir) $(DESTDIR)$(etcdir)
	$(INSTALL) -c -m 755 nwtutil $(DESTDIR)$(bindir)

uninstall:
	@rm -f $(DESTDIR)$(bindir)/nwtutil
clean:
	@rm -f *.o nwtutil *~
