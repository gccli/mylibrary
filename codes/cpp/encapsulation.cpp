#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>

using namespace std;

class A //encapsulation 
{
public:
    A(){}
    A(const char *str){
	int len = strlen(str);
	ptr = new char[len+1];
	memcpy(ptr, str, len);
	ptr[len] = 0;
    }
    virtual ~A()
    {
	printf("free memory\n");
	if (ptr) {
	    delete [] ptr;
	    ptr = NULL;
	}
    }

public:
    void show() const {
	if (ptr) printf("%s\n", ptr);
    }

    virtual int split(const char *str, const char *delim)
    {
    }

    operator char *() const { return ptr; } // implicit cast
protected:
    int  type;

private:
    char *ptr;
    char **vt;
    int   sz;
};

class D : public A
{
public:
    D(){}
    ~D(){}

    virtual int split(const char *str, const char *delim)
    {
	type = 1;
    }

private:
    int s;
};


int main(int argc, char *argv[])
{
    A a("hello world");
    puts(a);

    A *pA = new D;


    delete pA;

    return 0;
}
