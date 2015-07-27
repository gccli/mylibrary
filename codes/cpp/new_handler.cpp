#include <iostream>
using namespace std;


class NewHandlerHolder {
    public:
	explicit NewHandlerHolder(std::new_handler nh)
	    :handler(nh) {}
	~NewHandlerHolder() { std::set_new_handler(handler); }

    private:
	std::new_handler handler;
	NewHandlerHolder(const NewHandlerHolder&);
	NewHandlerHolder &operator=(const NewHandlerHolder&);
};

template<typename T> class NewHandlerSupport 
{
    public:
	static std::new_handler set_new_handler(std::new_handler p) throw();

	static void *operator new(std::size_t size) throw(std::bad_alloc);
    private:
	static std::new_handler ch; // current handler
};

template<typename T> std::new_handler NewHandlerSupport<T>::set_new_handler(std::new_handler p) throw()
{
    std::new_handler o = ch;
    ch = p;
    return o;
}

template<typename T> void* NewHandlerSupport<T>::operator new(std::size_t size) throw(std::bad_alloc)
{
    NewHandlerHolder h(std::set_new_handler(ch));

    printf("NewHandlerSupport<T>::operator new (%d) is called\n", size);

    return ::operator new(size);
}

template<typename T> std::new_handler NewHandlerSupport<T>::ch = 0;

class Widget : public NewHandlerSupport<Widget>
{
    public:
	Widget() { }

    private:
	char buffer[1024*1024];

};

char *prealloc = NULL;
void OutOfMemory()
{
    printf("out of memory\n");
    delete [] prealloc;
    sleep(1);
}

int main(int argc, char *argv[])
{
//    Widget::set_new_handler(OutOfMemory);

    prealloc = new char[1024*1024];
    for (int i; ;++i)
    {
	Widget *pw = new Widget;
//	char *p = new char[1024000];
    }

  return 0;
}
