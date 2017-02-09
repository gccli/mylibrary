#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/time.h>
#include <pcre.h>
#include <assert.h>


typedef struct my_regex {
    pcre            *code;
    const char      *pattern;
    struct my_regex *next;
    int              o[12];
    int              match;
} my_regex_t;

static my_regex_t *my_head;
int my_regex_add_pattern(const char *pattern)
{
    my_regex_t *temp;
    my_regex_t *node = calloc(sizeof(*node), 1);
    const char *str;
    int        erroffset;
    int        options = PCRE_EXTENDED;

    node->pattern = pattern;
    node->code = pcre_compile(
        pattern,          /* the pattern */
        options,          /* default options */
        &str,             /* for error message */
        &erroffset,       /* for error offset */
        NULL);            /* use default character tables */

    if (node->code == NULL) {
        printf("pattern %s compiled error - %s, column %d\n", pattern, str, erroffset);
        return 1;
    }

    if (my_head == NULL) {
        my_head = node;
    } else {
        temp = my_head;
        my_head = node;
        my_head->next = temp;
    }

    return 0;
}

void my_regex_add_patterns()
{
    int i;
    const char *patterns[] = {
        "t0k([^<\\n]+)([0-9]{5})e0k",
        "<.+?>",
        NULL
    };
    for(i=0; patterns[i]; i++) {
        if (my_regex_add_pattern(patterns[i])) {
            exit(1);
        }
    }
}

int main(int argc, char *argv[])
{
    const char *str;
    FILE       *fp;
    char        ibuf[2048];
    int         n;
    size_t      len;
    my_regex_t *node, *tmp;

    my_regex_add_patterns();
    if ((fp = fopen(argv[1], "r")) == NULL) {
        return 1;
    }

    while(!feof(fp)) {
        len = fread(ibuf, 1, sizeof(ibuf), fp);
        int offs = 0;

        for(; offs < (int)len; ) {
            // scan the page with all regex
            // http://www.pcre.org/original/doc/html/pcreapi.html#SEC17
            unsigned int min_index = (unsigned int) -1;
            for(tmp = NULL, node = my_head; node; node = node->next) {
                memset(node->o, 0, sizeof(node->o));

                node->match = pcre_exec(node->code, NULL, ibuf, len, offs, 0,
                                        node->o,
                                        sizeof(node->o)/sizeof(int));
                assert(node->match != 0);

                if (node->match < 0) {
                    if (node->match != PCRE_ERROR_NOMATCH) {
                        printf("rc:%-2d \n", node->match);
                    }
                    continue;
                }
                if (min_index > (unsigned int) node->o[0]) {
                    min_index = node->o[0];
                    tmp = node;
                }
            }

            if (tmp > 0) {
                int i;
                printf("\n(%04d,%04d) match regex (%s)\n", tmp->o[0], tmp->o[1], tmp->pattern);
                for(i=0; i<tmp->match; ++i) {
                    str = ibuf + tmp->o[2*i];
                    n = tmp->o[2*i+1] - tmp->o[2*i];
                    printf("  %.*s\n", n, str);
                }

                offs = tmp->o[1];
            } else {
                offs = len;
            }
        }
    }
    fclose(fp);

    return 0;
}
