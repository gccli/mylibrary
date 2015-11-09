#include <stdio.h>
#include <errno.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include "cipher.h"

/**
 * Generate random initialize vector, 8 bytes
 * dd if=/dev/random of=/tmp/x bs=8 count=1
 * hexdump -v -e '/1 "0x%02x,"' < /tmp/x ; echo
 */
static unsigned char default_iv[8] = {
    0x0e,0x27,0x2b,0x9c,0x90,0xdc,0xc6,0x62
};

#define CRYPT_LOGERR(str) do {                                    \
        char tmp_errstr[256];                                     \
        unsigned long tmp_errno;                                  \
        while((tmp_errno = ERR_get_error())) {                    \
            printf("%s %s\n", str,                                \
                   ERR_error_string(tmp_errno, tmp_errstr));      \
	}                                                         \
    } while(0)

void crypt_init_module()
{
    SSL_load_error_strings();
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    OpenSSL_add_all_ciphers();
}

int crypt_init(void *cipher_ctx, unsigned char *key, int key_len,
               int flags)
{
    EVP_CIPHER_CTX *ctx = (EVP_CIPHER_CTX *)cipher_ctx;

    if (key == NULL || key_len == 0) {
        printf("invalid key\n");
        return EINVAL;
    }

    EVP_CIPHER_CTX_init(ctx);
    const EVP_CIPHER *type = EVP_rc4();
    //const EVP_CIPHER *type = EVP_idea_cbc();


    EVP_CipherInit_ex(ctx, type, NULL, NULL, NULL, flags);
    EVP_CIPHER_CTX_set_key_length(ctx, key_len);
    EVP_CipherInit_ex(ctx, NULL, NULL, key, default_iv, flags);
    EVP_CIPHER_CTX_set_padding(ctx, 0); // disable padding

    return 0;
}

void crypt_destroy(void *cipher_ctx)
{
    EVP_CIPHER_CTX *ctx = (EVP_CIPHER_CTX *)cipher_ctx;
    EVP_CIPHER_CTX_cleanup(ctx);
}

void crypt_dump(EVP_CIPHER_CTX *ctx)
{
    int i, offs;
    char hexkey[4096] = {0};
    char hexiv[128] = {0};
    unsigned char *cipher = (unsigned char *)ctx->cipher_data;

    for(i=0,offs=0; i<EVP_CIPHER_CTX_key_length(ctx); ++i)
        offs += sprintf(hexkey+offs,"%.2x", cipher[i]);

    for(i=0,offs=0; i<EVP_CIPHER_CTX_iv_length(ctx); ++i)
        offs += sprintf(hexiv+offs, "%.2x", ctx->iv[i]);

    printf("Parameters:\n"
           "  key length : %d\n"
           "  iv         : %s(%d)\n"
           "  mode       : %ld\n"
           "  flags      : %ld\n"
           "  block-size : %d\n",
           EVP_CIPHER_CTX_key_length(ctx),
           hexiv, EVP_CIPHER_CTX_iv_length(ctx),
           EVP_CIPHER_CTX_mode(ctx),
           EVP_CIPHER_CTX_flags(ctx),
           EVP_CIPHER_CTX_block_size(ctx)
        );

// decrypt can be
    // openssl bf -d -in iname -K hexstring -iv hexstring
    // openssl enc -bf-ecb -nopad -d -in test1.enc -K hexstring -iv hexstring
}

/**
 * When @encrypt euqal to 1 for encryption and 0 for decryption
 */
static int crypt_do_crypt_file(void *cipher_ctx, FILE *in, FILE *out)
{
    EVP_CIPHER_CTX *ctx = (EVP_CIPHER_CTX *)cipher_ctx;

    unsigned char inbuf[1024];
    unsigned char outbuf[1024+EVP_MAX_BLOCK_LENGTH];
    int inlen, outlen;

    crypt_dump(ctx);

    for(;;) {
        inlen = fread(inbuf, 1, sizeof(inbuf), in);
        if(inlen <= 0) break;
        if(!EVP_CipherUpdate(ctx, outbuf, &outlen, inbuf, inlen)) {
            CRYPT_LOGERR("CipherUpdate");
            return EINVAL;
        }
        fwrite(outbuf, 1, outlen, out);
    }

    if(!EVP_CipherFinal_ex(ctx, outbuf, &outlen)) {
        CRYPT_LOGERR("CipherFinal");
        return EINVAL;
    }
    fwrite(outbuf, 1, outlen, out);

    return 0;
}

int crypt_decrypt(void *ctx, const char *infile, const char *outfile)
{
    int retval = 0;
    if (infile == NULL || infile[0] == 0 ||
        outfile == NULL || outfile[0] == 0) {
        printf("invlaid filename\n");
        return EINVAL;
    }

    FILE *fpin, *fpout;
    if ((fpin = fopen(infile, "rb")) == NULL) {
        printf("failed to open '%s'\n", infile);
        return errno;
    }
    if ((fpout = fopen(outfile, "wb")) == NULL) {
        printf("failed to open '%s'\n", outfile);
        fclose(fpin);
        return errno;
    }

    retval = crypt_do_crypt_file(ctx, fpin, fpout);

    fclose(fpin);
    fclose(fpout);

    return retval;
}

int crypt_encrypt(void *ctx, const char *infile, const char *outfile)
{
    int retval = 0;
    if (infile == NULL || infile[0] == 0 ||
        outfile == NULL || outfile[0] == 0) {
        printf("invlaid filename\n");
        return EINVAL;
    }

    FILE *fpin, *fpout;
    if ((fpin = fopen(infile, "rb")) == NULL) {
        printf("failed to open '%s'\n", infile);
        return errno;
    }
    if ((fpout = fopen(outfile, "wb")) == NULL) {
        printf("failed to open '%s'\n", outfile);
        fclose(fpin);
        return errno;
    }

    retval = crypt_do_crypt_file(ctx, fpin, fpout);

    fclose(fpin);
    fclose(fpout);

    return retval;
}
