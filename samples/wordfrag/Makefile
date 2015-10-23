TARGET:test

all:$(TARGET)
INC=-I. -I../../utils

test: wordfrag.cpp
	$(CXX) -Wall -g ${INC} -DOS_LINUX $^ -L. -lpthread -L. -lNLPIR  -o $@

clean:
	$(RM) $(TARGET) core
