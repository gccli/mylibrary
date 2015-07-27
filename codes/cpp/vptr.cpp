#include <stdio.h>
#include <iostream>
#include <string>
using namespace std;

// polymorphic base classes need declare a virtual destructor
// 

class AWOV
{
public:
    AWOV() {func();} // never call virtual functions during construction or destruction
    virtual ~AWOV() = 0;
    virtual void func() { printf("base\n"); }
//    virtual void func() = 0;
};
AWOV::~AWOV() { func(); }// never call virtual functions during construction or destruction

class Derived : public AWOV
{
public:
    void func(){ printf("derived\n"); }
};

int main(int argc, char *argv[])
{
    AWOV *p = new Derived;
    p->func();
    delete p;

    return 0;
}
