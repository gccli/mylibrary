all:mymap

CXXFLAGS=-I. -g -Wall
LDFLAGS=-lpthread -lrt -lssl -lcrypt

OBJS=main.o readwritelock.o feedback.o chunk.o


%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

%.o:../comm/%.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@


mymap:$(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS) 

clean:
	$(RM) *.o mymap
