#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <errno.h>
#include <ctype.h>

#include <map>
#include <string>

static std::map<std::string, int> html_entities = {
    { "lt", 60 },
    { "gt", 62 }
};

int html_find_entity(const char *entity)
{
    std::map<std::string, int>::iterator it = html_entities.begin();
    if (it == html_entities.end())
        return 0;

    return it->second;
}

int html_unescape(const char *src, size_t len, char **pdst, size_t *psz)
{
    const char      *start, *end;
    const char      *p, *q;
    int              i,j,l;
    int              ret = EINVAL;
    int              ent_id;
    char             entity[8] = {0};
    wchar_t         *dst;
    int              is_name;

    dst = (wchar_t *)calloc(len, sizeof(wchar_t));

    start = src;
    end = src+len;
    i = 0;
    while(start < end) {
        is_name = 0;
        p = strchr(start, '&');
        if (!p) {
            break;
        }
        p++;
        if (*p == '#') {
            p++;
        }

        q = strchr(p, ';');
        if (!q) {
            break;
        }
        start = q+1;
        l = q-p;
        if (l >= 8) {
            printf("HTML entity [%.*s] too long\n", l, p);
            goto error;
        }
        for(j=0; j<l; ++j) {
            if (!isdigit(p[j])) {
                is_name = 1;
            }
            entity[j] = p[j];
        }

        entity[j] = 0;

        if (is_name) {
            if ((ent_id = html_find_entity(entity)) == 0) {
                printf("HTML entity [%s] not define\n", entity);
                goto error;
            }
            dst[i++] = ent_id;
        } else {
            dst[i++] = atoi(entity);
        }
    }

    for(; start < end; ++start) {
        dst[i++] = *start;
    }

    *pdst = (char *)dst;
    *psz = i*sizeof(wchar_t);
    ret = 0;

error:
    if (ret) free(dst);
    return ret;
}

#include <iconv.h>
// g++ -std=c++11 -g -Wall html_unescape.cpp
int main(int argc, char *argv[])
{
    const char *str = "&#23596;&#38694;&#25493;&#29785;&#30777;&#37804;&#29448;";

    char *dst = NULL, *out, *p;
    size_t len = 0, outlen;

    html_unescape(str, strlen(str), &dst, &len);

    iconv_t cd = iconv_open("UTF-8", "WCHAR_T");

    outlen = len+1;
    out = (char *)calloc(outlen, 1);
    p = out;

    printf("convert %zu bytes\n", len);

    size_t sz = iconv(cd, &dst, &len, &p, &outlen);
    if (sz == (size_t) -1) {
        printf("error in convert %s\n", strerror(errno));
    }

    printf("%zu,%zu,%zu %s\n", outlen, sz, len, out);

    return 0;
}
