#include <mcheck.h>
#include <stdlib.h>
#include <stdio.h>


class A {
public:
   A() {}
   ~A(){}
private:
   char buff[1024];
};


int main(int argc, char *argv[])
{
   mtrace();
   A *a = new A;

   delete a;

   muntrace();   
   return 0;
}
