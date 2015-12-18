include ../Makefile.common

CFLAGS=-g -Wall -I$(INC) -fPIC
LDFLAGS=
ARFLAGS=rv

VPATH=.

SRCS =utilsock.cpp utilsockopt.cpp utiltime.cpp utilnet.cpp utilfile.c
SRCS+=hexdump.c
OBJS := $(patsubst %.cpp,%.o, $(SRCS))
OBJS := $(patsubst %.c,%.o, $(OBJS))
HEADERS := $(patsubst %.cpp,%.h, $(SRCS))
HEADERS := $(patsubst %.c,%.h, $(HEADERS))

TARGET=libcommutils.a
all:$(TARGET)

$(TARGET):$(OBJS)
	@mkdir -p $(@D)
	@echo -e "\033[32;1m#### Generate target $@ \033[0m"
	@$(AR) $(ARFLAGS) $@ $^

%.o:%.cpp
	$(CXX) $(CFLAGS) -c  $^ -o $@

%.o:%.c
	$(CC) $(CFLAGS) -c  $^ -o $@

clean:
	$(RM) -r $(OBJS) $(TARGET)

install:$(TARGET)
	ln -sf $(shell readlink -f $(TARGET)) $(libdir)
	@for i in $(HEADERS); do\
	  [ -f $$i ] && ln -sf $$(readlink -f $$i) $(prefix)/include;\
	done

uninstall:
	unlink $(libdir)/$(TARGET)
	@for i in $(HEADERS); do\
	  unlink  $(prefix)/include/$$i;\
	done
