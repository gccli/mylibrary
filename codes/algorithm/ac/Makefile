all:ac acsmx

ac:*.cpp
	$(CXX) -g -Wall -std=c++0x $^ -o $@

acsmx:*.c
	gcc -g -Wall -DACSMX_MAIN  $^ -o $@


clean:
	$(RM) ac acsmx core
