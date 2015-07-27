include ../../Makefile.common

CXXFLAGS= -g -Wall -I$(INC)
LDFLAGS= -L$(LIB) -lutilext

OBJFiles=  main.o
BINFiles=  testutil

all:$(BINFiles)

testutil:$(OBJFiles)
	@echo
	@echo -e "\t######## Now create executive file"
	$(CXX) -o $@ $^ $(LDFLAGS)


%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c  $^ -o $@

clean:
	$(RM) -r $(OBJFiles) $(BINFiles)
