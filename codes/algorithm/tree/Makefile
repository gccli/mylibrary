TARGET=btree

CXXFLAGS=
ifeq ($(opt),1)
CXXFLAGS=-O2
else
CXXFLAGS=-g -O0
endif

all:$(TARGET)

btree-src = $(wildcard *.cpp)
btree-obj = $(subst .cpp,.o,$(btree-src))

btree:$(btree-obj)
	$(CXX) -g $^ -o $@

clean:
	@find . -name "a.out" | xargs rm -f
	@find . -name "*.o" | xargs rm -f
	$(RM) $(TARGET) # $(btree-obj)
