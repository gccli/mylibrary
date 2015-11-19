#ifndef CRYPT_H__

#include <stdint.h>

/**
 * Initialize the cryptography module for both Symmetric symmetric and asymmetric
 * Call this function before any crypt operation
 */
int crypt_init();
int crypt_destroy();

/**
 * Generate @len bytes random secret key specified by @key
 */
int crypt_gen_key(void *key, size_t len);


/**
 * Create Encrypt handle, it encapsulate symmetric and RSA key
 * The RSA private key given by file @rsakey
 */
void *crypt_create(const char *rsakey);

/**
 * Used for transparent encrypt/decrypt
 */
int encrypt(const char *ifile, const char *ofile);
int decrypt(const char *ifile, const char *ofile);

#endif
