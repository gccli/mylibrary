#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
using namespace std;

class A 
{
public:
  ~A(){printf("~A");}
};
class B : public A 
{
public: 
  ~B(){printf("~B");}
};

int main(int argc, char *argv[])
{

  A *pA = new B; 
  delete pA; // only call A::~A();

  return 0;
}
