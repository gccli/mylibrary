#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>

#include "friend1.h"
#include "friend2.h"

void Friend1::make(Friend2 &f2)
{
    f2.val = val;
}

Friend1 Friend2::make(Friend1& f1)
{
    int x = Friend1::I_ID;

    f1.val = val;

    return f1;
}

int main(int argc, char *argv[])
{
    Friend1 f1(5);
    Friend2 f2(6);
    f2.make(f1);

    return 0;
}
