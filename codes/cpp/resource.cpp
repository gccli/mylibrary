#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>

using namespace std;

class A // RAII Resource Acquistion Is Initialization
{
public:
    A(){}
    A(const char *str){
	int len = strlen(str);
	ptr = new char[len+1];
	memcpy(ptr, str, len);
	ptr[len] = 0;
    }
    ~A()
    {
	printf("free memory\n");
	if (ptr) {
	    delete [] ptr;
	    ptr = NULL;
	}
    }
    void show() const {
	if (ptr) printf("%s\n", ptr);
    }

    operator char *() const { return ptr; } // implicit cast

private:
    char *ptr;
};




int main(int argc, char *argv[])
{
    A a("hello world");
    puts(a);

    A *pA = new A[10];
    delete [] pA;



    return 0;
}
