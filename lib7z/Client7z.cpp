#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libp7zip.h"

int  main(int numArgs, const char *args[])
{
	char c = args[1][0];
	if (c == 'a')
	{
		const char **flist = (const char ** ) calloc(4, sizeof(char *));
		flist[0] = args[3];

		Compress(args[2], flist);
	}
	else
	{
		int x = 0;
		if (c == 'l')
			x = 1;
		else if (c == 'x')
			x = 0;
		else
		{
			printf("incorrect command\n");
			return 1;
		}
		return Decompress(args[2], args[3], x);	
	}

	return 0;
}

