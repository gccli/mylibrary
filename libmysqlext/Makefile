include ../Makefile.common

CXXFLAGS=-g -Wall -I$(MYSQL_INC)
LDFLAGS=
ARFLAGS=rv

SRCDIR=mysqlext
OBJFiles=field.o row.o query.o conn.o 

PREFIX=$(MYLIBRARY)
LIBDIR=$(PREFIX)/lib
BINDIR=$(PREFIX)/bin
TARGET=$(LIBDIR)/libmysqlext.a

all:$(TARGET)

$(TARGET):$(OBJFiles)
	@mkdir -p $(@D)
	@echo -e "\033[32;1m#### Generate target $@ \033[0m"
	@$(AR) $(ARFLAGS) $@ $^ 

%.o:$(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c  $^ -o $@

clean:
	@$(RM) -r $(OBJFiles) $(TARGET)
