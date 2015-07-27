#include <stdio.h>
#include <iostream>
#include <string>
using namespace std;


class A
{
public:
    A(){};
    A(const A& a)
	:val(a.val)
	,str(a.str)
    {   }

    A& operator = (const A& a)
    {
	if (this == &a) return *this;
	
	val = a.val;
	return *this;
    }

    void init(int x, string y)
    {
	val = x;
	str = y;
    }

protected:
    int val;
    string str;
};

class D : public A
{
public:
    D(){t = time(0);};
    D(const D& d)
	:A(d)
	,t(d.t)
    {    }
    D& operator = (const D& d)
    {
	if (this == &d) return *this;
	A::operator=(d);
	t = d.t;

	return *this;
    }

    void show(){
	printf("val:%d str:%s time:%ld\n", val, str.c_str(), t);
    }

private:
    time_t t;
};

int main(int argc, char *argv[])
{
    D d, e;
    d.init(5, "hello");
    e = d;

    D f = d;

    d.show();
    e.show();
    f.show();

    return 0;
}
