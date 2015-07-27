#ifndef _FRIEND_2_H_
#define _FRIEND_2_H_

class Friend1;
class Friend2
{
    friend class Friend1;
  public:
  Friend2(int x)
      :val(x){}
    ~Friend2() { printf("%d\n", val); }
    Friend1 make(Friend1& f1); // ok. declare is ok
    void copy(Friend1 f1);// ok. declare is ok

  private:
    int val;
};

#endif
