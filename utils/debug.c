#include "debug.h"

#ifdef _TEST
int main(int argc, char *argv[])
{
    char buf[1024] = "hello world\r\n";
    print_string(buf, strlen(buf));

    return 0;
}
#endif
