#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>

using namespace std;
class Timer {
public:
    explicit Timer(int freq)    {}
    virtual void onTick() const {}
 
protected:
    void settimer() {
    }

private:
    void flush() {
    }


};

// private inheritance: is-implemented-in-terms-of
// need access protected member of base or need redefine virtual functions
class Widget: private Timer {
public:
    Widget() 
	:Timer(2)
    {}

    void func();

private:
    virtual void onTick() const {}

};
void Widget::func()
{
    settimer();
//    flush();
}

int main(int argc, char *argv[])
{
    Widget w;
    w.func();

    return 0;
}
