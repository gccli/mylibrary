#include <stdio.h>

int main(int argc, char *argv[])
{
	int width = 65, num = 10000;
	printf("%*d\n", width, num);
	printf("%2$*1$d\n", width, num);
	return 0;
}
