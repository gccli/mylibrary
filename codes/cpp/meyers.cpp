#include <stdio.h>
#include <iostream>
#include <string>

using namespace std;

class A
{
public:
  A(){};
  A(const char *str)
    :text(str) 
  {}

public:
  void func()
  {
    printf("size:%ld %s\n", sizeof(item), item);
    //    printf("NumItems:%x\n", &NumItems);
  }

  const char & operator[](size_t i) const
  {
    printf("const\n");
    return text[i];
  }
  char & operator[](size_t i)
  {
    printf("non-const\n");
    //return text[i];
    return const_cast<char &> (static_cast<const A&>(*this)[i]);
  }

  string text;

private:
  static const int NumItems = 10;
  static const double PI = 3.14;
  char item[NumItems];
};

int main(int argc, char *argv[])
{
  A a;
  //  a.func();
  a.text = "hello";
  printf("sizeof(a)=%ld\n", sizeof(a));
  printf("%c\n", a[4]);
  //  a[4] = 'l';
  //  printf("%c\n", a[4]);


  const A a1("world");
  printf("%c\n", a1[2]);
  //  a1[2] = 'd';


  return 0;
}
