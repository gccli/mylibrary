#ifndef URL_ESCAPE_H___
#define URL_ESCAPE_H___


char *url_escape(const char *string, size_t length);
int url_unescape(const char *string, size_t length, char **ostring, size_t *olen);

#endif
