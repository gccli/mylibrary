include ../Makefile.common

CXXFLAGS=-g -Wall -I$(INC)
LDFLAGS=
ARFLAGS=rv

SRCDIR=.
OBJFiles=logger.o  logqueue.o utilsock.o utilsockopt.o utiltime.o utilnet.o

PREFIX=$(HOME)/mylibrary
LIBDIR=$(PREFIX)/lib
BINDIR=$(PREFIX)/bin
TARGET=$(LIBDIR)/libutilext.a

all:$(TARGET)

$(TARGET):$(OBJFiles)
	@mkdir -p $(@D)
	@echo -e "\033[32;1m#### Generate target $@ \033[0m"
	@$(AR) $(ARFLAGS) $@ $^ 

%.o:$(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c  $^ -o $@

clean:
	@$(RM) -r $(OBJFiles) $(TARGET)
