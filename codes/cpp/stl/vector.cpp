#include <stdio.h>
#include <algorithm>
#include <vector>
#include <string>

using namespace std;

struct st
{
  char *p;
  int   q;
};

typedef vector

int main(int argc, char *argv[])
{
  int ary[] = {9, 7, 11, 29, 16, 1, 98, 51, 10, 5};
  vector<int> s(ary,ary+sizeof(ary)/sizeof(int));

  printf("sizeof(s) = %d\n", s.size());



  return 0;
}
