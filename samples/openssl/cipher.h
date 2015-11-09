#ifndef CRYTPO_H__
#define CRYTPO_H__

/**
 * Initialize the Symmetric-key cryptography module
 * Call this function before any crypt operation
 */
void crypt_init_module();

/**
 * Initialize cipher context @ctx (which type is EVP_CIPHER_CTX)
 * by secret @key and @encrypt flags 1 for encrypt 0 for decrypt
 */
int crypt_init(void *ctx, unsigned char *key, int key_len, int encrypt);

void crypt_destroy(void *ctx);

/**
 * decrypt file which name given by @in and write to output file @out
 */
int crypt_decrypt(void *ctx, const char *in, const char *out);


/**
 * encrypt file which name given by @in and write to output file @out
 */
int crypt_encrypt(void *ctx, const char *in, const char *out);


#endif
