#include "sslcommon.h"

BIO *SSLgetstderr()
{
    static BIO *SSL_bio_stderr = NULL;
    if (SSL_bio_stderr) {
        return SSL_bio_stderr;
    }

    SSL_bio_stderr = BIO_new_fp(stderr, BIO_NOCLOSE);
    return SSL_bio_stderr;
}
