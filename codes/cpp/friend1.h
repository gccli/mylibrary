#ifndef _FRIEND_1_H_
#define _FRIEND_1_H_

class Friend2;
class Friend1
{
    enum {
	I_ID,
	I_NAME
    };

    friend class Friend2;
  public:
  Friend1(int x)
      :val(x){}
    ~Friend1() { printf("%d\n", val); }
    void make(Friend2 &f2);
     
  private:
    int val;
};

#endif
