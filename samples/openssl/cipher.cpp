#include <stdio.h>
#include <errno.h>

#include "cipher.h"


/**
 * The initialize vector, max size 16 bytes, just hardcode it
 * Generate random initialize vector
 * dd if=/dev/random of=/tmp/x bs=16 count=1
 * hexdump -v -e '/1 "0x%02x,"' < /tmp/x ; echo
 */
static const unsigned char default_iv[EVP_MAX_IV_LENGTH] = {
    0xa9,0x1c,0xda,0x32,0x14,0x8f,0x16,0xa0,
    0x45,0xf3,0x06,0xfa,0xe2,0x85,0x8c,0xfb
};


cipher::cipher()
    :nopadding(0)
{
    memset(&cipher_ctx, 0, sizeof(cipher_ctx));
    sprintf(cipher_name, "aes-256-cbc");
}

cipher::cipher(const char *name, int pad)
    :nopadding(pad)
{
    memset(&cipher_ctx, 0, sizeof(cipher_ctx));
    sprintf(cipher_name, "aes-256-cbc");
}

cipher::~cipher()
{
    destroy();
    nopadding = 0;
}

int cipher::init(unsigned char *key, int key_len, const char *name, int flags)
{
    int enc;
    const EVP_CIPHER *cipher;
    EVP_CIPHER_CTX *ctx;
    if (key == NULL || key_len == 0) {
        printf("invalid key\n");
        return EINVAL;
    }
    if (name != NULL) {
        strncpy(cipher_name, name, sizeof(cipher_name));
    }

    cipher = EVP_get_cipherbyname(cipher_name);
    if (cipher == NULL) {
        printf("can not get cipher by name:%s", cipher_name);
        return EINVAL;
    }

    enc = 1;
    ctx = &cipher_ctx;

    EVP_CIPHER_CTX_init(ctx);
    if (flags & FLAG_DECRYPT) {
        enc = 0;
    }
    if (flags & FLAG_NO_PADDING) {
        nopadding = 1;
    }

    EVP_CipherInit_ex(ctx, cipher, NULL, NULL, NULL, enc);
    EVP_CIPHER_CTX_set_key_length(ctx, key_len);
    EVP_CipherInit_ex(ctx, NULL, NULL, key, default_iv, enc);


    return 0;
}

void cipher::destroy()
{
    EVP_CIPHER_CTX_cleanup(&cipher_ctx);
    memset(&cipher_ctx, 0, sizeof(cipher_ctx));
}

int cipher::encrypt_decrypt(BIO *in, BIO *out)
{
    int ret;
    int inlen, outlen;
    unsigned char inbuf[1024];
    unsigned char outbuf[1024+EVP_MAX_BLOCK_LENGTH];
    EVP_CIPHER_CTX *ctx = &cipher_ctx;

    if (nopadding) {
        EVP_CIPHER_CTX_set_padding(ctx, 0);
    }

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


int cipher::encrypt_decrypt(unsigned char *in, size_t inlen, BIO *out)
{
    int ret;
    int outlen;
    unsigned char inbuf[1024];
    unsigned char outbuf[1024+EVP_MAX_BLOCK_LENGTH];
    size_t total, len;
    EVP_CIPHER_CTX *ctx = &cipher_ctx;

    if (nopadding) {
        EVP_CIPHER_CTX_set_padding(ctx, 0);
    }

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

int cipher::dec_enc_file(const char *ifile, const char *ofile)
{
    int ret;
    BIO *in, *out;

    do {
        ret = EINVAL;
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

        if(!BIO_write_filename(out, (void *)ofile)) {
            CRYPT_LOGERR("BIO_write_filename");
            ret = EIO;
            break;
        }

        ret = encrypt_decrypt(in, out);
    } while(0);

    if (in) BIO_free(in);
    if (out) BIO_free(out);

    return ret;
}

int cipher::dec_enc_file(FILE *inf, FILE *outf)
{
    int ret;
    BIO *in, *out;

    do {
        ret = EINVAL;
        in = NULL;
        out = NULL;

        if ((in = BIO_new(BIO_s_file())) == NULL) {
            CRYPT_LOGERR("BIO_new");
            ret = ENOMEM;
        }
        if (BIO_set_fp(in, inf, BIO_NOCLOSE) <= 0) {
            CRYPT_LOGERR("BIO_set_fp(in)");
            break;
        }


        if ((out = BIO_new(BIO_s_file())) == NULL) {
            CRYPT_LOGERR("BIO_new");
            ret = ENOMEM;
        }
        if (BIO_set_fp(out, outf, BIO_NOCLOSE) <= 0) {
            CRYPT_LOGERR("BIO_set_fp(out)");
            break;
        }

        ret = encrypt_decrypt(in, out);
        if (ret) {
            break;
        }
    } while(0);

    if (in) BIO_free(in);
    if (out) BIO_free(out);

    return ret;
}

int cipher::dec_enc_file(FILE *inf, char **outp, size_t *outl)
{
    int ret;
    BIO *in, *mem;
    BUF_MEM *bptr = NULL;

    do {
        in = NULL;
        mem = NULL;

        if ((in = BIO_new(BIO_s_file())) == NULL) {
            CRYPT_LOGERR("BIO_new");
            ret = ENOMEM;
        }
        if (BIO_set_fp(in, inf, BIO_NOCLOSE) <= 0) {
            CRYPT_LOGERR("BIO_set_fp");
            break;
        }


/*
        int offs = (int) ftell(inf);
        if (offs != 0) {
            if (offs != BIO_seek(in, offs)) {
                printf("BIO_seek failed\n");
            }
        }
*/
        if ((mem = BIO_new(BIO_s_mem())) == NULL) {
            CRYPT_LOGERR("BIO_new");
            ret = ENOMEM;
            break;
        }

        ret = encrypt_decrypt(in, mem);
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

int cipher::dec_enc_buffer(unsigned char *in, size_t inlen, char **outp,
                           size_t *outl)
{
    int ret;

    BUF_MEM *bptr = NULL;
    BIO *mem = BIO_new(BIO_s_mem());
    if (mem == NULL) {
        CRYPT_LOGERR("BIO_new");
        return ENOMEM;
    }

    ret = encrypt_decrypt(in, inlen, mem);
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

int cipher::enc_dec_file(unsigned char *key, int key_len, const char *ifile,
                     const char *ofile, int dec)
{
    int ret;
    if (dec) dec = FLAG_DECRYPT;
    if ((ret = init(key, key_len, NULL, dec))) {
        destroy();
        return ret;
    }

    ret = dec_enc_file(ifile, ofile);
    destroy();
    return ret;
}

int cipher::enc_dec_file(unsigned char *key, int key_len, FILE *inf,
                     FILE *out, int dec)
{
    int ret;
    if (dec) dec = FLAG_DECRYPT;
    if ((ret = init(key, key_len, NULL, dec))) {
        destroy();
        return ret;
    }

    ret = dec_enc_file(inf, out);
    destroy();
    return ret;
}

int cipher::enc_dec_file(unsigned char *key, int key_len, FILE *inf,
                         char **outp, size_t *outl, int dec)
{
    int ret;
    if (dec) dec = FLAG_DECRYPT;
    if ((ret = init(key, key_len, NULL, dec))) {
        destroy();
        return ret;
    }

    ret = dec_enc_file(inf, outp, outl);
    destroy();
    return ret;
}


int cipher::enc_dec_buffer(unsigned char *key, int key_len, unsigned char *in,
                       size_t inlen, char **outp, size_t *outl, int dec)
{
    int ret;
    if (dec) dec = FLAG_DECRYPT;
    if ((ret = init(key, key_len, NULL, dec))) {
        destroy();
        return ret;
    }

    ret = dec_enc_buffer(in, inlen, outp, outl);
    destroy();
    return ret;
}

int cipher::enc_dec_stream(unsigned char *key, int key_len, BIO *in, BIO *out,
                           int dec)
{
    int ret;
    if (dec) dec = FLAG_DECRYPT;
    if ((ret = init(key, key_len, NULL, dec))) {
        destroy();
        return ret;
    }

    ret = encrypt_decrypt(in, out);
    destroy();
    return ret;
}


void cipher::dump_ctx()
{
    const char *mode;
    int i, m, offs;
    char tmpstr[128] = {0};
    EVP_CIPHER_CTX *ctx = &cipher_ctx;

    m = EVP_CIPHER_CTX_mode(ctx);
    switch(m) {
    case EVP_CIPH_STREAM_CIPHER:
        mode = "stream";
    case EVP_CIPH_ECB_MODE:
        mode = "ECB";
    case EVP_CIPH_CBC_MODE:
        mode = "CBC";
    case EVP_CIPH_CFB_MODE:
        mode = "CFB";
    case EVP_CIPH_OFB_MODE:
        mode = "OFB";
    default:
        mode = "Unknown";
    }

    if (EVP_CIPHER_CTX_iv_length(ctx) > 0) {
        for(i=0,offs=0; i<EVP_CIPHER_CTX_iv_length(ctx); ++i)
            offs += sprintf(tmpstr+offs, "%.2x", ctx->iv[i]);
        printf("  iv: %s(%d)", tmpstr, EVP_CIPHER_CTX_iv_length(ctx));
    }
    printf("  mode: %d(%s), block-size:%d\n", m, mode, EVP_CIPHER_CTX_block_size(ctx));
}
