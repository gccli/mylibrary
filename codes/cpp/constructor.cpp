#include <stdio.h>
#include <iostream>
#include <string>

using namespace std;

class A
{
public:
  A(int x)
    :val(x){ printf("constructor\n"); }
  int value() const { printf("%d\n", val); return val; }

private:
  int val;
};

template<typename T>
class NamedObject
{
public:
  NamedObject(string &s, const T& v)
    :nameval(s)
    ,val(v)
  {
  }
private:
  string &nameval; // reference and const member must implement assignment 
  const T val;
};



int main(int argc, char *argv[])
{
  A a(1);
  A b = a;
  a.value();
  b.value();

  string s = "hello";
  string t = "world";
  NamedObject<int> o(s, 1);
  NamedObject<int> p = o;
  NamedObject<int> q(s, 2);
  //  q = p;

  return 0;
}
