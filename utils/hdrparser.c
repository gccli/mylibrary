#include "parser.h"
#include <errno.h>
#include <string.h>
#include <ctype.h>

#define SP      ' '
#define CR      '\r'
#define CRLF    "\r\n"

#define iscrlf(p) (*(p) == '\r' && *((p)+1) == '\n')
#define notcrlf(p) (*(p) != '\r' && *((p) + 1) != '\n')

static const int parse_status_line(hdrparser_t *h, const char *body, size_t len,
                                   char **fields)
{
    int ret;
    char *p, *q;
    const char *end;

    do {
        ret = EINVAL;
        p = (char *)body;
        *fields = NULL;

        if ((end = strstr(body, CRLF)) == NULL) {
            break;
        }
        h->http_version.data = p;
        if ((q = strlchr(p, end, SP)) == NULL) {
            break;
        }
        h->http_version.len = q-p;
        if ((p = q+1) == end) {
            break;
        }

        h->u.rsp.status.data = p;
        if ((q = strlchr(p, end, SP)) == NULL) {
            break;
        }
        h->u.rsp.status.len = q-p;
        if ((p = q+1) == end) {
            break;
        }

        h->u.rsp.reason.data = p;
        if ((q = strlchr(p, end, CR)) == NULL) {
            break;
        }
        h->u.rsp.reason.len = q-p;
        if (q != end) {
            break;
        }

        ret = 0;
        *fields = (char *)end+2;
    } while(0);

    return ret;
}

static const int parse_request_line(hdrparser_t *h, const char *body, size_t len,
                                    char **fields)
{
    int ret;
    char *p, *q;
    const char *end;

    do {
        ret = EINVAL;
        p = (char *)body;
        *fields = NULL;

        if ((end = strstr(body, CRLF)) == NULL) {
            break;
        }
        h->u.req.method.data = p;
        if ((q = strlchr(p, end, SP)) == NULL) {
            break;
        }
        h->u.req.method.len = q-p;
        if ((p = q+1) == end) {
            break;
        }

        h->u.req.uri.data = p;
        if ((q = strlchr(p, end, SP)) == NULL) {
            break;
        }
        h->u.req.method.len = q-p;
        if ((p = q+1) == end) {
            break;
        }

        h->http_version.data = p;
        if ((q = strlchr(p, end, CR)) == NULL) {
            break;
        }
        h->http_version.len = q-p;
        if (q != end) {
            break;
        }

        ret = 0;
        *fields = (char *)end+2;
    } while(0);

    return ret;
}

int parser_header(const char *hdr, size_t size, hdrparser_t *h)
{
    int ret;
    char *p;
    const char *end;
    hdrfield_t *field;
    ret = EINVAL;
    end = hdr + size;

    if (!h || !hdr || *hdr == 0 || size == 0) {
        return ret;
    }

    if (memcmp(hdr, "HTTP", 4) == 0) {
        ret = parse_status_line(h, hdr, size, &p);
    } else {
        ret = parse_request_line(h, hdr, size, &p);
    }
    if (ret || p == NULL || isspace(*p)) {
        memset(h, 0, sizeof(*h));
        return ret;
    }

    h->field_size = 0;

    // Parse Header Fields Here
    do {
        ret = EINVAL;
        field = &(h->fields[h->field_size++]);

        field->key.data = p;
        while(p < end && *p != ':') p++;
        field->key.len = p - field->key.data;
        p++; // skip ':'

        if (p < end || iscrlf(p)) {
            break;
        }

        while(p != end && isspace(*p)) p++; // skip space
        if (p+1 >= end) {
            break;
        }

        field->val.data = p;
        while(p+1 <= end && notcrlf(p)) p++;
        field->val.len = p - field->val.data;

        if (!iscrlf(p)) {
            break;
        }
        p += 2;
#ifdef _DEBUG
        printf("  %.*s => %.*s\n", (int)field->key.len, field->key.data,
               (int)field->val.len, field->val.data );
#endif
        ret = 0;
        if (iscrlf(p)) break;
    } while(p < end);

    if (ret) {
        memset(h, 0, sizeof(*h));
    }

    return ret;
}
