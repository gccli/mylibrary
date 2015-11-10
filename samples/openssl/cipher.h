#ifndef CRYTPO_H__
#define CRYTPO_H__


typedef enum {
    FLAG_DECRYPT    = 0x01, // the context for encryption default
    FLAG_NO_PADDING = 0x02  // the padding is enabled by default
} crypt_flags_t;

/**
 * Initialize the Symmetric-key cryptography module
 * Call this function before any crypt operation
 */
void crypt_init_module();

/**
 * Initialize symmetric-key cryptography context @ctx for encrypt or decrypt.
 * The context @ctx type is openssl EVP_CIPHER_CTX. The secret key specified by
 * @key, and the key length is @key_len. The encrypt/decrypt algorithm is given
 * by @name, e.g. "DES3" or "AES-128-CBC", case insensitive; if @name is NULL,
 * the default algorithm "RC4" is given.
 * The context flags specified by @flags
 */
int crypt_init_ex(void *cipher_ctx, unsigned char *key, int key_len,
                  const char *name, int flags);
void crypt_destroy(void *ctx);

/**
 * Utility function for init encrypt/decrypt, use default setting
 */
int crypt_encrypt_init(void *ctx, unsigned char *key, int key_len,
                       const char *name);
int crypt_decrypt_init(void *ctx, unsigned char *key, int key_len,
                       const char *name);


/**
 * Utility function for encrypt/decrypt file or buffer
 */
int crypt_encrypt_file(void *ctx, char *in, char *out);
int crypt_decrypt_file(void *ctx, char *in, char *out);

int crypt_encrypt_buffer(void *ctx, unsigned char *in, char **out, int *outlen);
int crypt_decrypt_buffer(void *ctx, unsigned char *in, char **out, int *outlen);

#endif
