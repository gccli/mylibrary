#include "urlparser.h"

#include <errno.h>
#include <stdio.h>

static char *strlchr(char *p, const char *last, char c)
{
    while (p < last) {
        if (*p == c) {
            return p;
        }
        p++;
    }

    return NULL;
}

int parse_url(const char *url, size_t urlsz, urlparse_t *u)
{
    int ret;
    const char *last;
    char *p, *path, *query, *fragment;
    char *hp;               // host:port
    size_t hplen;           // host:port length
    char *rbracket = NULL;  // when host is IPv6 address

    ret = EINVAL;
    if (!u || !url || urlsz == 0) {
        return ret;
    }
    memset(u, 0, sizeof(*u));

    u->url.data = url;
    u->url.len = urlsz;
    u->schema.data = url;
    last = url + urlsz;

    if ((p = strstr(url, "://")) == NULL) {
        // Invalid URL, no schema
        goto error;
    }
    u->schema.len = p - url;
    p = p + 3;
    if (p == last) {
        goto error;
    }

    hp = p;
    if (hp[0] == '[') {
        // check IPv6 host
        hp += 1;
        if ((rbracket = strlchr(hp, last, ']')) == NULL) {
            goto error;
        }
    }
    if ((path = strlchr(hp, last, '/')) == NULL) {
        hplen = last - hp;
        if ((p = strlchr(hp, last, '#')) ||
            (p = strlchr(hp, last, '?'))) {
            hplen = p - hp;
            // ignore query and fragment string when no path, e.g.
            // https://tools.ietf.org/#section-1.1
        }
        goto parse_host_port;
    }
    hplen = path - hp;

    u->path.data = path;
    u->path.len = last - path;

    if (last - path == 1) {
        goto parse_host_port;
    }

    if ((query = strlchr(path, last, '?')) == NULL) {
        if ((fragment = strlchr(path, last, '#')) != NULL) {
            u->path.len = last - fragment;
            fragment += 1;
            if (last > fragment) {
                u->fragment.data = fragment; u->fragment.len = last - fragment;
            }
        }
        goto parse_host_port;
    }

    u->query.data = query+1;
    u->path.len = query - path;
    query += 1;
    if (last - query > 0) {
        u->query.len = last - query;
        if ((fragment = strlchr(query, last, '#')) != NULL) {
            u->query.len = fragment - query;
            fragment += 1;
            if (last > fragment) {
                u->fragment.data = fragment; u->fragment.len = last - fragment;
            }
        }
    }

parse_host_port:
    u->host.data = hp;
    last = hp + hplen;
    if (rbracket) {
        hplen = rbracket - hp - 1;
        // It's IPv6 host
        if ((p = strlchr(rbracket, last, ':')) != NULL) {
            p++;
            u->port.data = p;
            u->port.len = last -p;
        }
    } else {
        if ((p = strlchr(hp, last, ':')) != NULL) {
            hplen = p - hp;
            p++;
            u->port.data = p;
            u->port.len = last - p;
        }
    }
    u->host.len = hplen;

    ret = 0;

error:
    if (ret) {
        memset(u, 0, sizeof(*u));
    }
#ifdef _DEBUG
    int len;
    if (ret == 0) {
        printf("%.*s      [ok]\n", (int)u->url.len, u->url.data);
    } else {
        printf("%.*s   [error]\n", (int)urlsz, url);
    }

    if ((len = (int) u->schema.len) > 0) {
        printf("  schema: %.*s\n", len, u->schema.data);
    }
    if ((len = (int) u->host.len) > 0) {
        printf("  host:   %.*s\n", len, u->host.data);
    }
    if ((len = (int) u->port.len) > 0) {
        printf("  port:   %.*s\n", len, u->port.data);
    }
    if ((len = (int) u->path.len) > 0) {
        printf("  path:   %.*s\n", len, u->path.data);
    }
    if ((len = (int) u->query.len) > 0) {
        printf("  query:  %.*s\n", len, u->query.data);
    }
    if ((len = (int) u->fragment.len) > 0) {
        printf("  frag:   %.*s\n", len, u->fragment.data);
    }
#endif
    return ret;
}



#ifdef _URLPARSE_MAIN
/*
 * Test case
 * gcc -g -Wall urlparser.c -D_URLPARSE_MAIN -D_DEBUG
 * ./a.out url.txt  # each line in url.txt is a url
*/
#include <ctype.h>
size_t trim(char *p, char *pend) {
    for(; p<pend; ++p) {
        if (isspace(*p)) *p = 0;
        else break;
    }
    for(--pend; p<pend; --pend) {
        if (isspace(*pend)) *pend = 0;
        else break;
    }

    return pend - p + 1;
}

int main(int argc, char *argv[])
{
    FILE *fp;
    char *line = NULL;
    size_t len, size = 0;
    ssize_t rlen;

    urlparse_t url;
    fp = fopen(argv[1], "r");
    if (fp == NULL)
        exit(errno);

    while ((rlen = getline(&line, &size, fp)) != -1) {
        len = trim(line, line+rlen);
        parse_url(line, len, &url);
    }

    free(line);
    fclose(fp);
    return 0;
}
#endif
