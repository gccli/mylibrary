#include <stdio.h>

// Get pow(712,729) last 4 digits

int exp_mod(int x, int y, int m)
{
    int r = 1;
    while(y)
    {
	if ((y&0x1))
	    r = r*x % m;
	x = x*x % m;
	y /= 2;
    }
    return r%m;
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
	printf("%s x y m\n", argv[0]);
	return 0;
    }
    int x = atoi(argv[1]);
    int y = atoi(argv[2]);
    int m = atoi(argv[3]);
    printf("The last 4 digits of pow(%d,%d) is %d\n", x, y, exp_mod(x,y,m));

    return 0;
}
