#ifndef URL_ESCAPE_H___
#define URL_ESCAPE_H___

#ifdef __cplusplus
extern "C" {
#endif

char *url_escape(const char *string, size_t length);
int url_unescape(const char *string, size_t length, char **ostring, size_t *olen);


#ifdef __cplusplus
}
#endif

#endif
