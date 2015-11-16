#include <stdio.h>
#include <errno.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/rand.h>
#include "cipher.h"


/**
 * The initialize vector, max size 16 bytes, just hardcode it
 * Generate random initialize vector
 * dd if=/dev/random of=/tmp/x bs=16 count=1
 * hexdump -v -e '/1 "0x%02x,"' < /tmp/x ; echo
 */
static unsigned char default_iv[16] = {
    0xa9,0x1c,0xda,0x32,0x14,0x8f,0x16,0xa0,
    0x45,0xf3,0x06,0xfa,0xe2,0x85,0x8c,0xfb
};

void crypt_init_module()
{
    SSL_load_error_strings();
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    OpenSSL_add_all_ciphers();
}

int crypt_init_ex(void *cipher_ctx, unsigned char *key, int key_len,
                  const char *name, int flags)
{
    int enc;
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
    enc = 1;
    ctx = (EVP_CIPHER_CTX *)cipher_ctx;

    EVP_CIPHER_CTX_init(ctx);
    if (flags & FLAG_DECRYPT) {
        enc = 0;
    }

    EVP_CipherInit_ex(ctx, cipher, NULL, NULL, NULL, enc);
    EVP_CIPHER_CTX_set_key_length(ctx, key_len);
    EVP_CipherInit_ex(ctx, NULL, NULL, key, default_iv, enc);

    if (flags & FLAG_NO_PADDING) {
        EVP_CIPHER_CTX_set_padding(ctx, 0);
    }

    return 0;
}

int crypt_encrypt_init(void *ctx, unsigned char *key, int key_len,
                       const char *name)
{
    return crypt_init_ex(ctx, key, key_len, name, 0);
}

int crypt_decrypt_init(void *ctx, unsigned char *key, int key_len,
                       const char *name)
{
    return crypt_init_ex(ctx, key, key_len, name, FLAG_DECRYPT);
}

void crypt_destroy(void *cipher_ctx)
{
    EVP_CIPHER_CTX *ctx = (EVP_CIPHER_CTX *)cipher_ctx;
    EVP_CIPHER_CTX_cleanup(ctx);
}

static int crypt_internal(EVP_CIPHER_CTX *ctx, BIO *in, BIO *out)
{
    int ret;

    unsigned char inbuf[1024];
    unsigned char outbuf[1024+EVP_MAX_BLOCK_LENGTH];
    int inlen, outlen;

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

static int crypt_buffer(EVP_CIPHER_CTX *ctx, unsigned char *in, size_t inlen,
                        BIO *out)
{
    int ret;

    unsigned char inbuf[1024];
    unsigned char outbuf[1024+EVP_MAX_BLOCK_LENGTH];
    int outlen;

    size_t total, len;
    ret = 0;
    total = 0;
    while(inlen > 0) {
        len = inlen > sizeof(inbuf) ? sizeof(inbuf) : inlen;
        memcpy(inbuf, in + total, len);
        inlen -= len;
        total += len;

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

int dec_enc_file(void *cipher_ctx, char *ifile, char *ofile)
{
    int ret;
    BIO *in, *out;
    EVP_CIPHER_CTX *ctx = (EVP_CIPHER_CTX *)cipher_ctx;

    do {
        in = NULL;
        out = NULL;
        if ((in = BIO_new(BIO_s_file())) == NULL) {
            CRYPT_LOGERR("BIO_new");
            ret = ENOMEM;
            break;
        }

        if ((out = BIO_new(BIO_s_file())) == NULL) {
            CRYPT_LOGERR("BIO_new");
            ret = ENOMEM;
            break;
        }

        if(!BIO_read_filename(in, ifile)) {
            CRYPT_LOGERR("BIO_read_filename");
            ret = EIO;
            break;
        }

        if(!BIO_write_filename(out, ofile)) {
            CRYPT_LOGERR("BIO_write_filename");
            ret = EIO;
            break;
        }

        ret = crypt_internal(ctx, in, out);
    } while(0);

    if (in) BIO_free(in);
    if (out) BIO_free(out);

    return ret;
}

int dec_enc_buffer(void *c, unsigned char *in, size_t inlen,
                   char **outp, size_t *outl)
{
    int ret;
    EVP_CIPHER_CTX *ctx = (EVP_CIPHER_CTX *) c;
    BUF_MEM *bptr = NULL;
    BIO *mem = BIO_new(BIO_s_mem());
    if (mem == NULL) {
        CRYPT_LOGERR("BIO_new");
        return ENOMEM;
    }

    ret = crypt_buffer(ctx, in, inlen, mem);
    if (ret != 0) {
        BIO_free(mem);
        return ret;
    }

    BIO_get_mem_ptr(mem, &bptr);
    *outp = (char *)calloc(1, bptr->length);
    if (! *outp) {
        ret = ENOMEM;
    } else {
        memcpy(*outp, bptr->data, bptr->length);
        *outl = bptr->length;
    }

    BIO_free(mem);

    return 0;
}

int dec_enc_f2b(void *cipher_ctx, FILE *inf, char **outp, size_t *outl)
{
    int ret;
    BIO *in, *mem;
    BUF_MEM *bptr = NULL;
    EVP_CIPHER_CTX *ctx = (EVP_CIPHER_CTX *)cipher_ctx;
    int offs = 0;

    do {
        in = NULL;
        mem = NULL;

        if ((in = BIO_new(BIO_s_fd())) == NULL) {
            CRYPT_LOGERR("BIO_new");
            ret = ENOMEM;
        }
        if (BIO_set_fd(in, fileno(inf), BIO_NOCLOSE) <= 0) {
            CRYPT_LOGERR("BIO_set_fd");
            break;
        }

        offs = (int) ftell(inf);
        if (offs != 0) {
            if (offs != BIO_seek(in, offs)) {
                printf("BIO_seek failed\n");
            }
        }

        if ((mem = BIO_new(BIO_s_mem())) == NULL) {
            CRYPT_LOGERR("BIO_new");
            ret = ENOMEM;
            break;
        }

        ret = crypt_internal(ctx, in, mem);
        if (ret) {
            break;
        }

        BIO_get_mem_ptr(mem, &bptr);
        *outp = (char *)calloc(1, bptr->length);
        if (! *outp) {
            ret = ENOMEM;
        } else {
            memcpy(*outp, bptr->data, bptr->length);
            *outl = bptr->length;
        }
    } while(0);

    if (in) BIO_free(in);
    if (mem) BIO_free(mem);

    return ret;
}

int crypt_gen_key(void *key, size_t len)
{
    const char *randdev = "/dev/random";
    FILE *fp;

    if (RAND_bytes((unsigned char *)key, len) <= 0) {
        CRYPT_LOGERR("RAND_bytes");
        if ((fp = fopen(randdev, "rb")) == NULL) {
            printf("failed to open '%s': %s\n", randdev, strerror(errno));
            return errno;
        }
        if (len != fread(key, 1, len, fp)) {
            printf("can not read %zu bytes from '%s'", len, randdev);
            fclose(fp);
            return EINVAL;
        }
        fclose(fp);
    }

    return 0;
}
