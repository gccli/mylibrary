#ifndef URL_PARSER_H__
#define URL_PARSER_H__
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
#extern "C" {
#endif

// No need allocate memory
typedef struct url_string {
    const char *data;
    size_t      len;
} url_string_t;

typedef struct url_parse {
    url_string_t url;
    url_string_t schema;
    url_string_t host;
    url_string_t port;
    url_string_t path;
    url_string_t query;
    url_string_t fragment;
} urlparse_t;

int parse_url(const char *url, size_t urlsz, urlparse_t *u);

#ifdef __cplusplus
}
#endif

#endif
