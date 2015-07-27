#include <sasl.h>

extern int send_string(FILE *f, const char *s, int l);
extern int recv_string(FILE *f, char *buf, int buflen);
extern void saslerr(int why, const char *what, sasl_conn_t *conn);
extern void saslfail(int why, const char *what, sasl_conn_t *conn);
