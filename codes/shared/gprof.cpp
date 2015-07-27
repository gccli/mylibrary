#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "gprof_so.h"

int main(int argc, char *argv[])
{
    SProf *s = new SProf;
    s->call_so_func();

    return 0;
}
