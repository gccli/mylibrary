#ifndef HTTP_PARSER_H__
#define HTTP_PARSER_H__

#include "parser_defs.h"

#define MAX_HDR_FIELDS 32

typedef struct _hdr_field {
    pstr_t key;
    pstr_t val;
} hdrfield_t;

typedef struct _hdr_parser {
    pstr_t http_version;

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
} urlparser_t;

typedef struct _parser {
    urlparser_t *urlp;
    hdrparser_t *hdrp;
} parser_t;


int parse_url(const char *url, size_t size, urlparser_t *u);
int parser_header(const char *hdr, size_t size, hdrparser_t *h);

#endif
