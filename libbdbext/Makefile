CXXFLAGS=-g -Wall
LDFLAGS=-L. -L /usr/local/BerkeleyDB.5.1/lib -lbdb -lpthread -ldb

LIBBDBOBJS=bdbenv.o bdbcursor.o bdbquery.o
OBJS=main.o

all:libbdb.a dbtest

libbdb.a:$(LIBBDBOBJS)
	$(AR) r $@ $^

dbtest:$(OBJS) libbdb.a
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

clean:
	@rm -f dbtest *.o *~