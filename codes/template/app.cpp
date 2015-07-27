#include <iostream>
#include <string>
#include "InstanceId.h"

using namespace std;

class A
{
public:
    A()
    {
	cout<<id <<'\n';
    }
    ~A(){}
    bool operator ==(const A &a) const { return id == a.id; }


private:
    const InstanceId<A> id;    
};

class B
{
public:
    B()
    {
	cout<<id <<'\n';
    }
    ~B(){}

private:
    const InstanceId<B> id;    
};


InstanceIdDefinitions(A, "A");
InstanceIdDefinitions(B, "B");

int main(int argc, char *argv[])
{
    A a,b,c;
    if (a==b)
	B x;
    else
	B y;

    return 0;
}
