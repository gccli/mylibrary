#ifndef SSL_COMMON_H__
#define SSL_COMMON_H__

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>

#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/x509v3.h>

#include <pthread.h>

#define SSL_FAILURE 0
#define SSL_SUCCESS 1

typedef struct _CLIENT_SSL_CTX_extdata {
    BIO  *err;
    int  verify_ignore;   // ignore verify error
    int  verify_depth;    // max verify depth
    X509 *cert;           // server certificate sent
} CCTX_extdata_t;

typedef struct _SERVER_SSL_CTX_extdata {
    BIO  *err;
    const char *servername;
} SCTX_extdata_t;


int THREAD_setup(void);
int THREAD_cleanup(void);

#ifndef thread_id
#define thread_id syscall(__NR_gettid)
#endif

void SSLinit();
BIO *SSLgetstderr();
void SSLseeding(int size, const char *filename);

SSL_CTX *SSLnew_server_ctx(const char *cert, const char *key, char *pass);
SSL_CTX *SSLnew_client_ctx(const char *capath);

int BIO_nb_read(BIO *bio, char *start, int size);
int BIO_nb_write(BIO *bio, char *start, int size);

#define SSL_print_err(str) do {                                         \
        char tmp_errstr[256];                                           \
        unsigned long tmp_errno;                                        \
        while((tmp_errno = ERR_get_error())) {                          \
            BIO_printf(SSLgetstderr(), "%s %s\n", str,                  \
                       ERR_error_string(tmp_errno, tmp_errstr));        \
        }                                                               \
    } while(0)

extern int SSL_extdata_index;
extern int SSL_CTX_extdata_index;

#endif
