#include <stdio.h>
#include <pcre.h>
//#include <libxml2/libxml/HTMLParser.h>
#include <libxml/HTMLparser.h>


int main(int argc, char *argv[])
{
    pcre *re;
    const char *str;
    char buffer[2048];
    int  n;
    size_t len;
    int erroffset;

    const char *pattern  = "<div id=\"([^<]*)\".*</div>";
    re = pcre_compile(
        pattern,          /* the pattern */
        0,                /* default options */
        &str,             /* for error message */
        &erroffset,       /* for error offset */
        NULL);            /* use default character tables */

    htmlParserCtxtPtr ctxp = htmlNewParserCtxt();
    FILE *fp = fopen(argv[1], "r");
    if (!fp)
        return 1;


    while(!feof(fp)) {
        len = fread(buffer, 1, sizeof(buffer), fp);

        int offs = 0;
        for(; offs < len; ) {
            int i, rc;
            int ovec[30] = {0};
            rc = pcre_exec(
                re,             /* result of pcre_compile() */
                NULL,           /* we didn't study the pattern */
                buffer,         /* the subject string */
                len,            /* the length of the subject string */
                offs,           /* start at offset 0 in the subject */
                0,              /* default options */
                ovec,           /* vector of integers for substring information */
                30);            /* number of elements (NOT size in bytes) */

            if (rc == PCRE_ERROR_NOMATCH) break;
            else if (rc < 0) {
                printf("rc:%-2d ", rc);
            }
            for(i=0; i<30; i+=2) {
                if (ovec[i] > 0) {
                    str = buffer+ovec[i];
                    n = ovec[i+1]-ovec[i];
                    printf("(%d,%04d,%04d) %.*s\n", rc, ovec[i], ovec[i+1], n, str);
                    offs = ovec[i+1];
                    htmlCtxtReset(ctxp);
                    htmlCtxtReadMemory(ctxp,
                                       str,
                                       n,
                                       NULL,
                                       "UTF-8",
                                       0);
                } else {
                    break;
                }
            }
        }
    }
    fclose(fp);


    // '<div id="([^<]*)".*</div>'
    return 0;
}
