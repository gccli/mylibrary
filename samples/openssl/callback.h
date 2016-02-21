#ifndef SSL_CALLBACK_H__
#define SSL_CALLBACK_H__

#include "sslcommon.h"

typedef struct _TLS_extdata {
    BIO *err;
    int  ack;
} TLS_extdata;

int  SSLservername_cb(SSL *s, int *ad, void *arg);
int  SSLverify_callback(int ok, X509_STORE_CTX *ctx);
void SSLinfo_callback(const SSL *s, int where, int ret);


#endif
