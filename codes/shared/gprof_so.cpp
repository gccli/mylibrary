#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "gprof_so.h"

void SProf::call_so_func()
{
    func1();    
    func2();
    func3();
}


void SProf::func1()
{
    printf("\n");
    for(int i=0; i<200*1024*1024; ++i) {
	printf(".");
    }
    printf("\n");
}
void SProf::func2() 
{
    for(int i=0; i<50*1024*1024; ++i) {
	printf(".");
    }
    printf("\n");
}

void SProf::func3() 
{
    for(int i=0; i<100*1024*1024; ++i) {
	printf(".");
    }
    printf("\n");
}


