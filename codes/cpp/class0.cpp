#include <stdio.h>
#include <iostream>
#include <string>

using namespace std;

class A 
{
public:
};
class B : public A {};

class A1 
{
public:
  virtual ~A1(){};

public:
  virtual void func();

private:
  char buffer[1];  // align 4 in 32 bit

};
class B1 : public A1 {};

class Empty
{
public:
    Empty() {
        puts("default");
    }
    Empty(const Empty &em) {
        puts("copy");
    }
    Empty& operator=(const Empty &em) {
        puts("assignment");
    }
    ~Empty() {
        puts("destructor");
    }
};

void func(Empty e)
{
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        return 0;
    }

    printf("sizeof(A)=%d, sizeof(B)=%d\n", sizeof(A), sizeof(B));
    printf("sizeof(A1)=%d, sizeof(B1)=%d\n", sizeof(A1), sizeof(B1));

  Empty e1;
  Empty e2(e1);

  e2 = e1;
  Empty e3 = e2;

  func(e2);
  



  return 0;
}
