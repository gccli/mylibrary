#ifndef CIPHER_H__
#define CIPHER_H__

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>

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

class cipher {
public:
    cipher();
    cipher(const char *name, int pad);
    virtual ~cipher();

/**
 * Initialize symmetric-key cryptography context for encrypt or decrypt.
 * The secret key specified by @key, and the key length is @key_len.
 * The encrypt/decrypt algorithm is given by @name, e.g. "rc4" or
 * "AES-128-CBC", case insensitive; if @name is NULL, the default algorithm
 * "aes-256-cbc" is given. The context flags specified by @flags
 */
    int init(unsigned char *key, int key_len, const char *name, int flags);
    void destroy();

    int encrypt_decrypt(BIO *in, BIO *out);
    int encrypt_decrypt(unsigned char *in, size_t inlen, BIO *out);


/**
 * Utility function for encrypt/decrypt file or buffer
 */
    int enc_dec_file(unsigned char *key, int key_len, const char *ifile,
                     const char *ofile, int dec=0);
    int enc_dec_file(unsigned char *key, int key_len, FILE *inf,
                     FILE *out, int dec=0);
    int enc_dec_file(unsigned char *key, int key_len, FILE *inf,
                     char **outp, size_t *outl, int dec=0);
    int enc_dec_buffer(unsigned char *key, int key_len, unsigned char *in,
                       size_t inlen, char **outp, size_t *outl, int dec=0);
    int enc_dec_stream(unsigned char *key, int key_len, BIO *in, BIO *out,
                       int dec=0);

    void dump_ctx();

protected:
/**
 * Utility function for encrypt/decrypt file or buffer
 * These functions should init context before call
 */
    int dec_enc_file(const char *ifile, const char *ofile);
    int dec_enc_file(FILE *inf, FILE *out);
    int dec_enc_file(FILE *inf, char **outp, size_t *outl);
    int dec_enc_buffer(unsigned char *in, size_t inlen,
                       char **outp, size_t *outl);

private:
    int nopadding;
    char cipher_name[32];
    EVP_CIPHER_CTX cipher_ctx;
};


#endif
