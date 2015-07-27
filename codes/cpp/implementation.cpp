#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <string>
#include <vector>
#include <stdexcept>
#include <tr1/memory>
using namespace std;

class A 
{
public:
    A(){}
    A(const string s)
	:name(s){}

    A(const A &a){
	printf("base copy\n");
    }

    void setname(const char *s) { name = s; }
    const string &getname()
    {
	return name;
    }

    virtual void dosomething() {
	printf("%s base do something\n", name.c_str());
    }

private:
    string name;
};

class B : public A  
{
public:
    B(){}
private:
    long timestamp;
};

class C 
{
public:
    C(){}
private:
    std::vector<int> ids;
};

class Binary
{
public:
    Binary()
	:size(0)
	,ptr(NULL)
    {}
    Binary(const Binary &bin)
    {
	if ((ptr = malloc(bin.size)) == NULL)
	    return ;
	memcpy(ptr, bin.ptr, bin.size);
	size = bin.size;
    }

    ~Binary() {
	printf("binary:%p freed\n", ptr);

	if (ptr) { 
	    free(ptr); ptr = NULL; size = 0; 
	}
    }

public:
    size_t size;
    void  *ptr;
};

class D : public B, public C
{
public:
    D(){ pImage = std::tr1::shared_ptr<Binary>(new Binary); }
    void dosomething() 
    {
	static_cast<A>(*this).dosomething();  // wrong: just a copy
//	A::dosomething();
	printf("derived do something\n");
    }

    void ChangeImage(const Binary &image)
    {
	printf("image address : %p change to ", pImage->ptr);
	pImage.reset(new Binary(image));
	printf("%p\n", pImage->ptr);
    }

private:
    double start;

    std::tr1::shared_ptr<Binary> pImage;
};


void show(const A *a)
{
    A *x = const_cast<A*>(a);
    printf("x = %s\n", x->getname().c_str());
    x->setname("world");
    printf("x = %s\n", x->getname().c_str());
}

int main(int argc, char *argv[])
{
    A a("hello");
    show(&a);

    D d;
    d.setname("zhonghua");

    A *pa = &d;
    B *pb = &d;
    C *pc = &d;
    D *pd = &d;
    printf("single object maybe have more one address\n");
    printf("pa:%p\npb:%p\npc:%p\npd:%p\n", pa, pb, pc, pd);

    d.dosomething();

    Binary image;
    image.ptr = malloc(1024);
    image.size = 1024;

    d.ChangeImage(image);

//    throw logic_error("hello, world");

    return 0;
}
