#ifndef CRYTPO_H__
#define CRYTPO_H__

#define CRYPT_LOGERR(str) do {                                    \
        char tmp_errstr[256];                                     \
        unsigned long tmp_errno;                                  \
        while((tmp_errno = ERR_get_error())) {                    \
            printf("%s %s\n", str,                                \
                   ERR_error_string(tmp_errno, tmp_errstr));      \
	}                                                         \
    } while(0)

// AES_256 key length in bytes
#define AES_KEY_LEN 32


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

int dec_enc_file(void *cipher_ctx, char *ifile, char *ofile);
int dec_enc_buffer(void *c, unsigned char *in, size_t inlen,
                   char **outp, int *outl);
int dec_enc_file_to_buffer(void *ctx, FILE *in, char **out, int *outl);


/**
 * Generate @len bytes random secret key specified by @key
 */
int crypt_gen_key(void *key, size_t len);


#endif
