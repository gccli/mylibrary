#ifndef __MY_COMM_H__
#define __MY_COMM_H__

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <memory>
#include <tr1/memory>

using namespace std;
using namespace tr1;

// try to replace between shared_ptr and shared_ptr, the former will core dump, the latter is ok
// shared_ptr in namespace std::tr1
// sed -i 's/shared_ptr/auto_ptr/' *.h *.cpp
// sed -i 's/auto_ptr/shared_ptr/' *.h *.cpp

typedef int (*CALLBACK_FUNC)(int, const char *, void *, void *);
typedef int (*SOAPI_FUNC)(int, void *);

void printlog(int level,
	      const char *filename,
	      const char *funcname,
	      int line,
	      const char *tag,
	      const char *, ...);


#define Mylog(...)\
    printlog(1,__FILE__,__FUNCTION__,__LINE__,"",	\
	       ##__VA_ARGS__)

int P7ZDecompress(const char *inname, const char *outdir, int flags);

class AbsCommClass {
public:
    virtual ~AbsCommClass() {}
    virtual void show() {
	Mylog("this is %p\n", this);
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

