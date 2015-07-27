include ../../Makefile.common

CXXFLAGS= -g -Wall -I$(MYSQL_INC) -I$(MYSQLEXT)
LDFLAGS= -L$(LIB) -lmysqlext -lmysqlclient

OBJFiles=  main.o
BINFiles=  testmysql

all:$(BINFiles)

testmysql:$(OBJFiles)
	@echo
	@echo -e "\t######## Now create executive file"
	$(CXX) -o $@ $^ $(LDFLAGS)


%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c  $^ -o $@

clean:
	$(RM) -r $(OBJFiles) $(BINFiles)
