#ifndef CRYPT_H__
#define CRYPT_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct crypt_ctx;


/**
 * Initialize the Openssl cryptography module
 * Call this function before any crypt operation
 */
int crypt_init();
int crypt_destroy();

/**
 * Generate @len bytes random secret key specified by @key
 */
int crypt_gen_key(void *key, size_t len);

/**
 * Generate RSA key and export to file @keypath
 */
int crypt_rsa_genkey(const char *keypath);

/**
 * Create cryptography @handle, which encapsulate Symmetric and RSA key
 * The RSA private key given by file @rsakey
 */
int crypt_create(struct crypt_ctx **handle, const char *rsakey);
int crypt_create_ex(struct crypt_ctx **handle, const char *rsakey, int flags);
void crypt_free(struct crypt_ctx *);

/**
 * Used for transparent encrypt/decrypt
 */
int encrypt_f(struct crypt_ctx *handle, const char *ifile, const char *ofile);
int decrypt_f(struct crypt_ctx *handle, const char *ifile, const char *ofile);
int encrypt_s(struct crypt_ctx *handle, BIO *in, BIO *out);


#ifdef __cplusplus
}
#endif

#endif
