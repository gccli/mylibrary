#include <iostream>

using namespace std;


class A {
public:
  virtual void func() = 0;
};

class B : public A {
public:
  B();
  ~B();

  void func();

private:
  char c;
};

B::B()
{
  A::A();
  c = 0;
}

void B::func()
{
}

int main(int argc, char *argv[])
{

  B b;
  
  return 0;
}
