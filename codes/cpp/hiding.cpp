#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>

// Avoid hiding inherited names

using namespace std;

class B //encapsulation 
{
public:
    B(){}

    virtual void func1() = 0;
    virtual void func1(int x) {}
    void func2() {}
    void func2(int y) {}

private:
    int   sz;
};

class D : public B
{
public:
    D(){}
    ~D(){}

    // <--------
    using B::func1;
    using B::func2;
    // -------->

    void func1() {}
    void func2() {};

private:
    int s;
};


int main(int argc, char *argv[])
{
    D d;

    d.func1();
    d.func1(1); // wrong, D::func1  hiding A::func1(int)
    d.func2();
    d.func2(1); // wrong, D::func2  hiding A::func2(int)

    return 0;
}
