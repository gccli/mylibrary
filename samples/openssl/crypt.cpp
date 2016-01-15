#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/rand.h>

#include "cipher.h"
#include "rsakey.h"
#include "crypt_common.h"
#include "crypt.h"

#ifdef __cplusplus
extern "C" {
#endif

static uint64_t magic_number = 0xfeeeeeeffeeeeeef;
struct crypt_ctx {
    cipher *ciph;
    rsakey *rsa;
};

typedef struct crypt_ctx crypt_ctx_t;

/**
 * RSA default passphrase, used for encrypt private key
 * when export and load
 */

static unsigned char rsa_pass[] = {
    0x7b,0xfe,0x9f,0x3d,0x9e,0xc6,0x06,0x7e,
    0x70,0x35,0xe9,0x6a,0x1b,0x6e,0x94,0
};


int crypt_init()
{
    SSL_load_error_strings();
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    OpenSSL_add_all_ciphers();

    return 0;
}

int crypt_destroy()
{
    ERR_free_strings();
    EVP_cleanup();

    return 0;
}

int crypt_gen_key(void *key, size_t len)
{
    FILE *fp;
    const char *dev = "/dev/random";

    if (RAND_bytes((unsigned char *)key, len) <= 0) {
        CRYPT_LOGERR("RAND_bytes");
        if ((fp = fopen(dev, "rb")) == NULL) {
            printf("failed to open '%s': %s\n", dev, strerror(errno));
            return errno;
        }
        if (len != fread(key, 1, len, fp)) {
            printf("can not read %zu bytes from '%s'", len, dev);
            fclose(fp);
            return EINVAL;
        }
        fclose(fp);
    }

    return 0;
}

int crypt_create_ex(struct crypt_ctx **ctxp, const char *rsakeyf, int flags)
{
    int ret;

    crypt_ctx_t *ctx;
    const char *pubkey = "/tmp/key.pub";

    do {
        ret = ENOMEM;

        ctx = (crypt_ctx_t *)calloc(1, sizeof(*ctx));
        if (!ctx) break;

        ctx->ciph = new cipher("aes-256-cbc", flags);
        if (!ctx->ciph) {
            break;
        }

        ctx->rsa = new rsakey();
        if (!ctx->rsa) {
            break;
        }
        if ((ret = ctx->rsa->load_key(rsakeyf, rsa_pass))) {
            printf("failed to load rsa key\n");
            break;
        }

        if ((ret = ctx->rsa->export_key(pubkey))) {
            break;
        }
        if ((ret = ctx->rsa->load_pubkey(pubkey))) {
            break;
        }

        *ctxp = ctx;
        ret = 0;
    } while(0);

    if (ret) {
        crypt_free(ctx);
        ctx = NULL;
    }

    return ret;
}

int crypt_create(crypt_ctx_t **ctxp, const char *private_key)
{
    return crypt_create_ex(ctxp, private_key, 0);
}

void crypt_free(crypt_ctx_t *ctx)
{
    if (ctx) {
        if (ctx->rsa) delete ctx->rsa;
        if (ctx->ciph) delete ctx->ciph;
        ctx->rsa = NULL;
        ctx->ciph = NULL;
        free(ctx);
    }
}

int encrypt_s(crypt_ctx_t *ctx, BIO *in, BIO *out)
{
    int ret;
    unsigned char secret[AES_KEY_LEN];
    unsigned char buffer[512], *pout;

    size_t len, outl = 0;

    do {
        ret = EINVAL;
        if (crypt_gen_key(secret, sizeof(secret)))
            break;

        ret = ctx->rsa->enc_dec(secret, sizeof(secret), &pout, &outl);
        if (ret) break;

        // header length
        len = sizeof(magic_number) + 4 + outl;

        // construct header
        ul2buf(magic_number, buffer);
        memcpy(buffer+sizeof(magic_number), &len, 2);
        memcpy(buffer+sizeof(magic_number)+4, pout, outl);
        free(pout);

        if ((ret = BIO_write(out, buffer, len)) != (int)len) {
            printf("BIO_write %d bytes, not equal %zu", ret, len);
            if (ret == -1) {
                CRYPT_LOGERR("BIO_write");
            }
            ret = EIO;
            break;
        }

        ret = ctx->ciph->enc_dec_stream(secret, sizeof(secret), in, out);
    } while(0);

    return ret;
}


int encrypt_f(crypt_ctx_t *ctx, const char *ifile, const char *ofile)
{
    int ret;
    FILE *fpi, *fpo;

    unsigned char secret[AES_KEY_LEN];
    unsigned char buffer[512], *pout;

    size_t len, outl = 0;

    do {
        ret = EINVAL;
        fpi = NULL;
        fpo = NULL;

        if ((fpi = fopen(ifile, "rb")) == NULL)
            break;
        if ((fpo = fopen(ofile, "wb")) == NULL)
            break;

        if (crypt_gen_key(secret, sizeof(secret)))
            break;

        ret = ctx->rsa->enc_dec(secret, sizeof(secret), &pout, &outl);
        if (ret) break;

        // header length
        len = sizeof(magic_number) + 4 + outl;

        // construct header
        ul2buf(magic_number, buffer);
        memcpy(buffer+sizeof(magic_number), &len, 2);
        memcpy(buffer+sizeof(magic_number)+4, pout, outl);
        free(pout);

        fwrite(buffer, 1, len, fpo);

        ret = ctx->ciph->enc_dec_file(secret, sizeof(secret), fpi, fpo);
    } while(0);

    if (fpi) fclose(fpi);
    if (fpo) fclose(fpo);

    return ret;
}

int decrypt_f(crypt_ctx_t *ctx, const char *ifile, const char *ofile)
{
    int ret;

    FILE *fpi, *fpo;
    unsigned char buffer[512], *pout;
    size_t len, outl = 0;

    do {
        ret = EINVAL;
        fpi = NULL;
        fpo = NULL;

        if ((fpi = fopen(ifile, "rb")) == NULL)
            break;
        if ((fpo = fopen(ofile, "wb")) == NULL)
            break;

        fread(buffer, 1, sizeof(magic_number), fpi);
        if (buf2ul(buffer) != magic_number) {
            printf("magic number mismatch\n");
            break;
        }

        fread(buffer, 1, 4, fpi);
        memcpy(&len, buffer, 2);
        len = len & 0xffff;
        len = len - sizeof(magic_number) - 4;

        fread(buffer, 1, len, fpi);

        ret = ctx->rsa->enc_dec(buffer, len,  &pout, &outl, 1);
        if (ret) break;
        if (outl != AES_KEY_LEN) {
            printf("key length mismatch\n");
            break;
        }

        ret = ctx->ciph->enc_dec_file(pout, outl, fpi, fpo, 1);
    } while(0);

    if (fpi) fclose(fpi);
    if (fpo) fclose(fpo);

    return ret;
}

int crypt_rsa_genkey(const char *keypath)
{
    int ret;
    EVP_PKEY *pk = NULL;
    ret = rsakey::generate(&pk);
    if (pk == NULL) {
        return ret;
    }
    if (rsakey::export_key(pk, keypath, rsa_pass)) {
        EVP_PKEY_free(pk);
        return ENOENT;
    }

    EVP_PKEY_free(pk);
    return 0;
}

#ifdef __cplusplus
}
#endif



/*------------------------------------------------------*
 *               For Test                               *
 *------------------------------------------------------*/
#ifdef _CRYPT_MAIN
extern "C" {
#include <utils/time.h>
#include <utils/file.h>
}
#include <assert.h>

int main(int argc, char *argv[])
{
    int ret, index;
    int dec = 0, nopad = 0, sym = 0;
    unsigned char symkey[16] = {'s', 'e', 'c', 'r', 'e', 't', 'k', 'e', 'y'};
    static struct option long_options[] = {
        {"sym", 0, 0, 0},
        {0, 0, 0, 0}
    };

    const char* optlist = "dn";
    while (1){
        int c = getopt_long(argc, argv, optlist, long_options, &index);
        if (c == EOF) break;
        switch (c) {
        case 'd':
            dec = 1;
            break;
        case 'n':
            nopad = 1;
            break;
        case 0:
            if (strcmp(long_options[index].name, "sym") == 0) {
                sym = 1;
            }
            break;
        default:
            printf("usage: %s [-dn] [--sym] in out\n", argv[0]);
            exit(0);
        }
    }

    crypt_init();

    // Generate
    if (access("key", R_OK) != 0) {
        EVP_PKEY *pk = NULL;
        rsakey::generate(&pk);
        if (pk == NULL) return 1;
        if (rsakey::export_key(pk, "key", rsa_pass))
            return ENOENT;
    }

    crypt_ctx_t *ctx;
    ret = crypt_create_ex(&ctx, "key", nopad);
    if (ret != 0) {
        printf("failed to create context\n");
        return 1;
    }
    if (sym) { // for symmetrical encrypt/decrypt
        if (dec) {
            ret = ctx->ciph->enc_dec_file(symkey, sizeof(symkey), argv[optind],argv[optind+1], 1);
        } else {
            ret = ctx->ciph->enc_dec_file(symkey, sizeof(symkey), argv[optind],argv[optind+1], 0);
        }
        goto done;
    }

    if (dec) {
        ret = decrypt_f(ctx, argv[optind],argv[optind+1]);
    } else {
        ret = encrypt_f(ctx, argv[optind],argv[optind+1]);
    }

done:
    crypt_free(ctx);
    crypt_destroy();

    return ret;
}

#endif
// Verify
// in=cipher.h; ./crypt $in /tmp/enc; ll $in /tmp/enc;./crypt /tmp/enc /tmp/dec -d; diff $in /tmp/dec
//
// decrypt file
// openssl bf -d -in iname -K hexstring -iv hexstring
// openssl enc -bf-ecb -nopad -d -in test1.enc -K hexstring -iv hexstring
