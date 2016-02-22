#include <sys/syscall.h>

#include "sslcommon.h"
#include "callback.h"

#define SSL_FAILURE 0
#define SSL_SUCCESS 1

//////////////////////////////////////////////////
// OpenSSL thread-safe
static pthread_mutex_t *ssl_mutex = NULL;
static void thread_lock(int mode, int n, const char * file, int line)
{
    if (mode & CRYPTO_LOCK)
        pthread_mutex_lock(&ssl_mutex[n]);
    else
        pthread_mutex_unlock(&ssl_mutex[n]);
}

int THREAD_setup(void)
{
    int i, mutex_count = CRYPTO_num_locks();
    if ((ssl_mutex = calloc(mutex_count, sizeof(pthread_mutex_t))) == NULL)
        return SSL_FAILURE;

    for (i=0 ;i<mutex_count; ++i)
        pthread_mutex_init(&ssl_mutex[i], NULL);

    CRYPTO_set_id_callback(pthread_self);
    CRYPTO_set_locking_callback(thread_lock);

    return SSL_SUCCESS;
}

int THREAD_cleanup(void)
{
    CRYPTO_set_id_callback(NULL);
    CRYPTO_set_locking_callback(NULL);
    if (ssl_mutex == NULL)
        return SSL_FAILURE;
    int i;
    for (i = 0; i < CRYPTO_num_locks(); i++)
        pthread_mutex_destroy(&ssl_mutex[i]);
    free(ssl_mutex);
    ssl_mutex = NULL;

    return SSL_SUCCESS;
}
//////////////////////////////////////////////////

int SSL_extdata_index;
int SSL_CTX_extdata_index;

// Common Functions
void SSLinit()
{
    SSL_library_init();

    //THREAD_setup();
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    OpenSSL_add_all_ciphers();

    SSL_extdata_index = SSL_get_ex_new_index(0, "SSL_extdata", NULL, NULL, NULL);
    SSL_CTX_extdata_index = SSL_CTX_get_ex_new_index(0, "SSL_extdata", NULL, NULL, NULL);
}

void SSLcleanup()
{
    ERR_free_strings();
    EVP_cleanup();
}

void SSLseeding(int size, const char *filename)
{
    if (access(filename, F_OK) != 0)
    {
        RAND_load_file("/dev/random", size);
        RAND_write_file(filename);
    }

    int nb = RAND_load_file(filename, -1);
    printf("Seeded the PRNG with %d byte(s) of data from %s\n", nb, filename);
}

static int SSLpasswd_cb(char *buf, int size, int rwflag, void *password)
{
    if (password == NULL) {
        password = getpass("Enter the password: ");
    }

    strncpy(buf, (char *)password, size);
    buf[size] = 0;

    return strlen(buf);
}

SSL_CTX *SSLnew_server_ctx(const char *cert, const char *keyfile, char *pass)
{
    SSL_CTX *ctx = SSL_CTX_new(TLSv1_method());

    SSL_CTX_set_default_passwd_cb(ctx, SSLpasswd_cb);
    SSL_CTX_set_default_passwd_cb_userdata(ctx, pass);

    if (SSL_CTX_use_certificate_chain_file(ctx, cert) <= 0)
    {
        SSL_print_err("use certificate");
        goto error;
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, keyfile, SSL_FILETYPE_PEM) <= 0)
    {
        SSL_print_err("use private key");
        goto error;
    }
    if (SSL_CTX_check_private_key(ctx) <= 0)
    {
        SSL_print_err("check private key");
        goto error;
    }

    SSL_CTX_set_tlsext_servername_callback(ctx, SSLservername_cb);
    //SSL_CTX_set_tlsext_servername_arg(ctx, &tlsext);

    SSL_CTX_set_info_callback(ctx, SSLinfo_callback);

    return ctx;

error:
    SSL_CTX_free(ctx);
    return NULL;
}

SSL_CTX *SSLnew_client_ctx(const char *capath)
{
    SSL_CTX *ctx = SSL_CTX_new(TLSv1_method());
    CCTX_extdata_t *extdata;

    extdata = (CCTX_extdata_t *)calloc(1, sizeof(*extdata));
    extdata->verify_depth = 5;
    extdata->verify_ignore = 1;
    extdata->err = SSLgetstderr();

    SSL_CTX_set_ex_data(ctx, SSL_CTX_extdata_index, extdata);

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, SSLverify_callback);
    SSL_CTX_set_info_callback(ctx, SSLinfo_callback);

    // set default locations for trusted CA certificates
    if (SSL_CTX_load_verify_locations(ctx, NULL, capath) <= 0) {
        SSL_print_err("load verify location:");
        goto error;
    }

    if (!SSL_CTX_set_default_verify_paths(ctx)) {
        SSL_print_err("set default verify path:");
        goto error;
    }

    SSL_CTX_set_tlsext_servername_callback(ctx, SSLservername_cb);
    SSL_CTX_set_tlsext_servername_arg(ctx, extdata);

    return ctx;

error:
    SSL_CTX_free(ctx);
    return NULL;
}


int BIO_nb_write(BIO *bio, char *start, int size)
{
    int offs = 0;
    int len, left = size;
    while (offs < size) {
        len = BIO_write(bio, start+offs, left);
        if (len <= 0) {
            if (BIO_should_retry(bio)) continue;
            else {
                SSL_print_err("BIO_write");
                return -1;
            }
        }
        offs += len;
        left -= len;
    }

    return offs;
}

int BIO_nb_read(BIO *bio, char *start, int size)
{
    int offs = 0;
    int len, left = size;
    do {
        len = BIO_read(bio, start+offs, left);
        if (len == 0) {
            offs = -1;
            break;
        }
        if (len < 0) {
            if (!BIO_should_read(bio)) {
                SSL_print_err("BIO_read");
                return -1;
            }
        }
        offs += len;
        left -= len;
    } while(0);

    return offs;
}
