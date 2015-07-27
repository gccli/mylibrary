#ifndef __MY_COMM_H__
#define __MY_COMM_H__

#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <memory>
#include <tr1/memory>

using namespace std;
using namespace tr1;

// try to replace between shared_ptr and shared_ptr, the former will core dump, the latter is ok
// shared_ptr in namespace std::tr1
// sed -i 's/shared_ptr/auto_ptr/' *.h *.cpp
// sed -i 's/auto_ptr/shared_ptr/' *.h *.cpp

typedef int (*CALLBACK_FUNC)(int, void *, void *);

extern int SO1API(int cmd, void *param);
extern int SO2API(int cmd, void *param);


class AbsCommClass {
public:
    virtual ~AbsCommClass() {}
    virtual void show() {
	printf("this is %p\n", this);
    }
};

class CommClass : public AbsCommClass {
public:
    ~CommClass() { }

    static CommClass *geti();

private:
    static shared_ptr<CommClass> m_cc; 
};

#endif

