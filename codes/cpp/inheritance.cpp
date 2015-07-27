#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>

// Differentiate between inheritance of interface and inheritance of implementation

using namespace std;

class Shape {
public:
    virtual void drew() = 0; // inherit interface only
    virtual void error(const char *str) = 0; // declaration part represent interface

    virtual void rotate() {}; // inherit interface and implementation
    unsigned int id() { return m_id; } // inherit implementation, invariant override specialization


    // Never redefine a function's inherited default parameter value
    virtual int circumference(int flags = 1)
    {
	printf("default circumference %d\n", flags);
    }

private:
    unsigned int m_id;
};

// definition part represent default behaviour
void Shape::error(const char *str) 
{
    printf("error:%s\n", str); 
}


class Oval : public Shape
{
public:
    Oval() {}

    void drew() { printf("drew oval\n"); }
    void error(const char *str) { Shape::error(str); }


    // Never redefine a function's inherited default parameter value
    virtual int circumference(int flags = 2)
    {
	printf("oval's circumference %d\n", flags);
    }
};

class Triangle : public Shape
{
public:
    Triangle() {}
    void drew() { printf("drew triangle\n"); }
    void error(const char *str) { printf("triangle: %s\n", str); }

    // Never redefine a function's inherited default parameter value
    virtual int circumference(int flags)
    {
	printf("triangle's circumference %d\n", flags);
    }
};

int main(int argc, char *argv[])
{
    Oval o;
    Triangle t;

    Shape *p;
    Shape *po = &o;
    Shape *pt = &t;

    o.drew();
    t.drew();

// call function defined by derived class, using default parameter of base class mean while.
    po->circumference(); 
    pt->circumference();

    p = pt;

    p->circumference(55);

    return 0;
}
