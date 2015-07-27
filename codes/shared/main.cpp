#include "comm.h"

static int so1callback(int cmd, void *para1, void *para2)
{
	printf("'so1' callback called, para1:%p para2:%p\n", para1, para2);
	return 0;
}

static int so2callback(int cmd, void *para1, void *para2)
{
	printf("'so2' callback called, para1:%p para2:%p\n", para1, para2);
	return 0;
}

int main(int argc, char *argv[])
{
    printf("SO1API:%p, SO2API:%p, so1callback:%p, so2callback:%p\n"
	   ,SO1API, SO2API, so1callback, so2callback);

    SO1API(0, (void *)so1callback);
    SO2API(0, (void *)so2callback);

    SO1API(1, NULL);
    SO2API(1, NULL);


    printf("exit main()\n");
    fflush(stdout);
    return 0;
}
