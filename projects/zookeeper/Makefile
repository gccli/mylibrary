TARGET=zkclient

INCL=-I $(HOME)/projects/zookeeper-3.4.6/src/c/include -I $(HOME)/projects/zookeeper-3.4.6/src/c/generated
LIBS=$(HOME)/projects/zookeeper-3.4.6/src/c/.libs/libzookeeper_mt.a -lpthread

zkclient:zkclient.cpp
	$(CXX) -g -Wall $(INCL) $^ $(LIBS) -o $@

clean:
	$(RM) $(TARGET)
