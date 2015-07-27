#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>

int myscandir(const char *dirp, int width)
{
    struct dirent **namelist;
    int n;

    char path[1024];


    n = scandir(dirp, &namelist, 0, alphasort);
    if (n < 0) {
	printf("failed to scandir '%s'\n", dirp);
	return -1;
    }

    while (n--) {
	snprintf(path, sizeof(path), "%s/%s", dirp, namelist[n]->d_name);
	if (namelist[n]->d_type == DT_DIR) {
	    if (namelist[n]->d_name[0] == '.') continue;
	    printf("%s\n", path);
	    myscandir(path, width+2);
	} else {
	    printf("%s\n", path);
	}
	free(namelist[n]);
    }
    free(namelist);
    return 0;
}


int main(int argc, char *argv[])
{
    myscandir(argv[1], 0);
    return 0;
}
