#include <sys/types.h>
#include <regex.h>
#include <stdio.h>
#include <string>
#include <dirent.h>
#include <assert.h>
using namespace std;


int main(int argc, char *argv[])
{
    int        i, rc, len;
    regex_t    re;
    regmatch_t match[4];
    char       error[256] = {0};

    if (argc < 3) {
        printf("Usage: %s pattern string\n", argv[0]);
        return 1;
    }

    if ((rc = regcomp(&re, argv[1], REG_EXTENDED)) != 0) {
        regerror(rc, &re, error, sizeof(error));
        printf("Compile regexp[%s] error - %s\n", argv[1], error);
        return 1;
    }

    rc = regexec(&re, argv[2], sizeof(match)/sizeof(regmatch_t), match, 0);
    if (rc != 0) {
        regerror(rc, &re, error, sizeof(error));
        printf("Exec regexp[%s] error - %s\n", argv[1], error);
        return 1;
    }
    for(i=0; i<4; ++i) {
        if (match[i].rm_eo < 0)
            break;

        len = match[i].rm_eo - match[i].rm_so;
        printf("%.*s\n", len, argv[2]+match[i].rm_so);
    }

    regfree(&re);

    return 0;
}
