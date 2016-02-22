#ifndef SSL_CALLBACK_H__
#define SSL_CALLBACK_H__

#include "sslcommon.h"

int  SSLservername_cb(SSL *s, int *ad, void *arg);
int  SSLverify_callback(int ok, X509_STORE_CTX *ctx);
void SSLinfo_callback(const SSL *s, int where, int ret);

#endif
