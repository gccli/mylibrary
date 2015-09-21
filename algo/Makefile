TARGET=dec2x
CFLAGS=
ifeq ($(opt),1)
CFLAGS=-02
else
CFLAGS=-g -O0
endif

all:$(TARGET)
dec2x:covert_decimal.cpp
	$(CXX) -g $^ -o $@

clean:
	@find . -name "a.out" | xargs rm -f
	@find . -name "*.o" | xargs rm -f
	$(RM) $(TARGET)
