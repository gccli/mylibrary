#include <stdio.h>
#include <string.h>
void func(const int &a, int const &b)
{
    // a eq b
}

int main(int argc, char *argv[])
{
    char buf[128] = {0};
    char buf1[128];
    strcat(buf, "helloword, gcc\n");
    strcat(buf1, "const char * and char * const\n");

    const char *s = buf; // 指向常量char类型的指针
    char * const s1 = buf; // 常量指针
    const char *const s2 = buf;
    printf ("%s%s\n", s, s1);

    s = buf1;
    //s1 = NULL; // error: read-only variable s1
    // *s = 'x'; // error: read-only location
    *s1 = 'x';    
    printf ("%s%s%x\n", s, s1, s1);

    int x = 104, y = 105;
    func(x,y);
    printf ("%d %d\n", x, y);
    
    return 0;
}
