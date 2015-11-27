#ifndef HTTP_DATA_H__
#define HTTP_DATA_H__

#include <stdlib.h>
#include <ctype.h>

#define MAX_HDR_FIELDS 32

#ifdef _DEBUG
#include <stdio.h>
#include <string.h>
#define __FN__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define debug_print(msg,...)                                            \
    fprintf(stderr, "%s:%d: " msg, __FN__, __LINE__, ##__VA_ARGS__)
#else
#define debug_print(msg, ...) ((void)0)
#endif


#ifdef __cplusplus
extern "C" {
#endif

// No need to allocate memory
typedef struct _parser_string {
    const char *data;
    size_t      len;
} pstr_t;

typedef struct _hdr_field {
    pstr_t key;
    pstr_t val;
} hdrfield_t;

typedef struct _hdr_parser {
    int valid;
    int response;
    union {
        /**
         * Request-Line = Method SP Request-URI SP HTTP-Version CRLF
         */
        struct {
            pstr_t method;
            pstr_t uri;
        } req;
        /**
         * Response
         * Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
         */
        struct {
            pstr_t status;
            pstr_t reason;
        } rsp;
    } u;
    pstr_t http_version;

    size_t field_size;
    hdrfield_t fields[MAX_HDR_FIELDS];
} hdrparser_t;

typedef struct _url_parser {
    pstr_t url;
    pstr_t schema;
    pstr_t host;
    pstr_t port;
    pstr_t path;
    pstr_t query;
    pstr_t fragment;

    int valid;
} urlparser_t;

typedef struct _parser {
    urlparser_t *urlp;
    hdrparser_t *hdrp;

    void *data; // point to user data, used for callback
} parser_t;


// F U N C T I O N S
static inline char *strlchr(char *p, const char *last, char c)
{
    while (p < last) {
        if (*p == c) {
            return p;
        }
        p++;
    }

    return NULL;
}


static inline void str2lower(char *p, size_t len)
{
    char *e = p+len;
    while(p < e) {
        if (*p >= 'A' && *p <= 'Z') *p = *p + 'a' - 'A';
        p++;
    }
}

static inline size_t strtrim(char *p, char *pend)
{
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

int parse_url(const char *url, size_t size, urlparser_t *u);
int parse_header(const char *hdr, size_t size, hdrparser_t *h);


#ifdef __cplusplus
}
#endif

#endif
