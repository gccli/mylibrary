include ../../../Makefile.common

TARGET=mq shm shmc shms
CXXFLAGS=-g -Wall -I$(INC)

all:$(TARGET)

mq:mq.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ -lrt  

shm:shm_mmap.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ -lrt

shmc:shm_client.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ -lrt

shms:shm_server.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ -lrt

clean:
	@$(RM) $(TARGET)
