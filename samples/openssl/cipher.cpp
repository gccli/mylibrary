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

int crypt_init_ex(void *cipher_ctx, const char *name,
                  unsigned char *key, int key_len, int flags)
{
    const EVP_CIPHER *cipher;
    EVP_CIPHER_CTX *ctx;
    if (key == NULL || key_len == 0) {
        printf("invalid key\n");
        return EINVAL;
    }
    if (name == NULL) {
        cipher = EVP_rc4();
    } else {
        cipher = EVP_get_cipherbyname(name);
        if (cipher == NULL) {
            CRYPT_LOGERR("get cipher");
        }
    }

    if (cipher == NULL) {
        return EINVAL;
    }

    ctx = (EVP_CIPHER_CTX *)cipher_ctx;

    EVP_CIPHER_CTX_init(ctx);

    EVP_CipherInit_ex(ctx, cipher, NULL, NULL, NULL, flags);
    EVP_CIPHER_CTX_set_key_length(ctx, key_len);
    EVP_CipherInit_ex(ctx, NULL, NULL, key, default_iv, flags);
    EVP_CIPHER_CTX_set_padding(ctx, 0); // disable padding

    return 0;
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
    char hexiv[32] = {0};

    printf("CIPHER Settings:\n");
//    printf("  name:    %s\n", SSL_CIPHER_get_name(ctx->cipher));
    if (EVP_CIPHER_CTX_iv_length(ctx) > 0) {
        for(i=0,offs=0; i<EVP_CIPHER_CTX_iv_length(ctx); ++i)
            offs += sprintf(hexiv+offs, "%.2x", ctx->iv[i]);
        printf("  iv:    %s(%d)", hexiv, EVP_CIPHER_CTX_iv_length(ctx));
    }
    printf("  mode:    %ld\n",EVP_CIPHER_CTX_mode(ctx));
    printf("  block-size:    %d\n",EVP_CIPHER_CTX_block_size(ctx));
}

static int crypt_internal(EVP_CIPHER_CTX *ctx, BIO *in, BIO *out)
{
    int ret;

    unsigned char inbuf[1024];
    unsigned char outbuf[1024+EVP_MAX_BLOCK_LENGTH];
    int inlen, outlen;

    crypt_dump(ctx);

    ret = 0;
    while(!BIO_eof(in)) {
        inlen = BIO_read(in, inbuf, sizeof(inbuf));
        if (inlen <= 0) {
            if (inlen < 0) {
                CRYPT_LOGERR("BIO read");
                ret = EIO;
            }
            break;
        }

        if(!EVP_CipherUpdate(ctx, outbuf, &outlen, inbuf, inlen)) {
            CRYPT_LOGERR("CipherUpdate");
            return EINVAL;
        }
        if (BIO_write(out, outbuf, outlen) != outlen) {
            CRYPT_LOGERR("BIO write");
            ret = EIO;
        }
    }
    if (ret) {
        return ret;
    }

    if(!EVP_CipherFinal_ex(ctx, outbuf, &outlen)) {
        CRYPT_LOGERR("CipherFinal");
        return EINVAL;
    }

    if (BIO_write(out, outbuf, outlen) != outlen) {
        CRYPT_LOGERR("BIO final write");
        ret = EIO;
    }

    return ret;
}

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

int crypt_encrypt_ex(EVP_CIPHER_CTX *ctx,
                     const char *infile, char *outfile)
{
    int ret;
    BIO *in, *out;

    do {
        in = NULL;
        out = NULL;
        if ((in =  BIO_new(BIO_s_file())) == NULL) {
            CRYPT_LOGERR("BIO_new");
            ret = ENOMEM;
            break;
        }

        if ((out = BIO_new(BIO_s_file())) == NULL) {
            CRYPT_LOGERR("BIO_new");
            ret = ENOMEM;
            break;
        }

        if(!BIO_read_filename(in, infile)) {
            CRYPT_LOGERR("BIO_read_filename");
            ret = EIO;
            break;
        }

        if(!BIO_write_filename(out, outfile)) {
            CRYPT_LOGERR("BIO_write_filename");
            ret = EIO;
            break;
        }
    } while(0);

    ret = crypt_internal(ctx, in, out);


    if (in) BIO_free(in);
    if (out) BIO_free(out);

    return ret;
}
