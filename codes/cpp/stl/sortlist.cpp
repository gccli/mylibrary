#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <list>
#include <string>

using namespace std;

#define NUMBER 10
struct SExample 
{
  unsigned int   m_id;
  string         m_name;
  time_t         m_time;
  void*          m_private;

  SExample()
    :m_id(0)
    ,m_time(0)
    ,m_private(0)
  {}
  void print()
  {
    struct tm *ptm = localtime(&m_time);
    char strtime[64] = "\0";
    sprintf(strtime, "%02d:%02d:%02d", ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
    printf("id:%-3d name:%-12s time:%s\n", m_id, m_name.c_str(), strtime);
  }
};

inline bool operator < (const SExample src, const SExample dst)
{
  return src.m_id < dst.m_id;
}
class SExampleCmp
{
public:
  bool operator()(SExample* src, SExample* dst)
  {
    return src->m_id < dst->m_id;
  }
};
class SExampleCmpByName
{
public:
  bool operator()(SExample* src, SExample* dst)
  {
    int val = strcmp(src->m_name.c_str(), dst->m_name.c_str());
    return (val < 0);
  }
  bool operator()(SExample src, SExample dst)
  {
    int val = strcmp(src.m_name.c_str(), dst.m_name.c_str());
    return (val < 0);
  }
};

void print_list(list<SExample> l)
{
  for (list<SExample>::iterator i = l.begin(); i != l.end(); ++i)
    i->print();
  printf("\n");
}
void print_list2(list<SExample *> l)
{
  for (list<SExample *>::iterator i = l.begin(); i != l.end(); ++i) 
    (*i)->print();
  printf("\n");
}

int main(int argc, char *argv[])
{
  int i;
  int random_number[NUMBER] = 
    {
      31,64,99,1,45,51,36,78,87,2
    };
  char random_string[NUMBER][20] = 
    {
      "ephemeral", "skeptical", "besiege", "marvelous", "notorious",
      "discretion","paranoia",  "solicit", "ingredient","resist"
    };

  list<SExample>   x;
  list<SExample* > y;

  time_t t = time(NULL);
  for (i=0; i<NUMBER; ++i) 
    {
      SExample  a;
      SExample* b = new SExample;
      b->m_id = a.m_id = random_number[i];
      b->m_name = a.m_name = random_string[i];
      b->m_time = a.m_time = t + a.m_id;

      x.push_back(a);
      y.push_back(b);
    }

  printf("-------- sort list<SExample> --------\n");
  print_list(x);
  x.sort(); // CALL inline bool operator < (const SExample src, const SExample dst)
  print_list(x);
  x.sort(SExampleCmpByName()); // CALL SExampleCmpByName::operator()(SExample src, SExample dst)
  print_list(x);
  
  printf("-------- sort list<SExample* > --------\n");

  print_list2(y);
  y.sort(SExampleCmp()); // CALL SExampleCmp::operator()(SExample* src, SExample* dst)
  print_list2(y);
  y.sort(SExampleCmpByName());// CALL SExampleCmpByName::operator()(SExample* src, SExample* dst)
  print_list2(y);

  char c = getc(stdin);
  return c;
}

