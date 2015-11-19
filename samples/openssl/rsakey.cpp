#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include "cipher.h"
#include "rsakey.h"

rsakey::rsakey()
    :pk_pri(NULL)
    ,pk_pub(NULL)
{
}

rsakey::~rsakey()
{
}

int rsakey::generate(EVP_PKEY **pk)
{
    int ret;
    EVP_PKEY_CTX *ctx;

    do {
        ret = EINVAL;
        ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
        if (!ctx) {
            CRYPT_LOGERR("EVP_PKEY_CTX_new_id");
            break;
        }

        if (EVP_PKEY_keygen_init(ctx) <= 0) {
            CRYPT_LOGERR("EVP_PKEY_keygen_init");
            break;
        }

        if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0) {
            CRYPT_LOGERR("EVP_PKEY_CTX_set_rsa_keygen_bits");
            break;
        }

        if (EVP_PKEY_keygen(ctx, pk) <= 0) {
            CRYPT_LOGERR("EVP_PKEY_keygen");
            break;
        }
        ret = 0;
    } while(0);

    return ret;
}

int rsakey::export_pubkey(EVP_PKEY *pk, const char *pubkey)
{
    BIO *out = NULL;

    out = BIO_new_file(pubkey, "w");
    if (!out) {
        CRYPT_LOGERR("BIO_new_file");
        return EPERM;
    }

    if (PEM_write_bio_PUBKEY(out, pk) <= 0) {
        CRYPT_LOGERR("PEM_write_bio_PUBKEY");
        BIO_free(out);
        return EINVAL;
    }
    BIO_free(out);

    return 0;
}

int rsakey::export_key(EVP_PKEY *pk, const char *prikey, unsigned char *pass)
{
    int ret, len;
    const EVP_CIPHER *cipher;
    BIO *out = NULL;

    // export private key
    out = BIO_new_file(prikey, "w");
    if (!out) {
        CRYPT_LOGERR("BIO_new_file");
        return EPERM;
    }

    len = 0;
    if (pass) {
        len = strlen((char *)pass);
    }
    if (len > 0) {
        cipher = EVP_aes_128_cbc();
    } else {
        cipher = NULL;
        pass = NULL;
    }
    ret = PEM_write_bio_PrivateKey(out, pk, cipher, NULL, 0, NULL, pass);
    if (ret <= 0) {
        CRYPT_LOGERR("PEM_write_bio_PrivateKey");
        BIO_free(out);
        return EINVAL;
    }
    BIO_free(out);

    return 0;
}

int rsakey::export_key(const char *ofile, int pub, unsigned char *pass)
{
    int ret;
    if (!pk_pri) {
        return EINVAL;
    }
    if (pub) {
        ret = export_pubkey(pk_pri, ofile);
    } else {
        ret = export_key(pk_pri, ofile, pass);
    }

    return ret;
}

int rsakey::load_key(const char *ifile, unsigned char *pass)
{
    int ret = 0;
    EVP_PKEY *pk = NULL;
    BIO *in = NULL;

    do {
        in = BIO_new(BIO_s_file());
        if (!in) {
            CRYPT_LOGERR("BIO_new: failed to create BIO");
            ret = ENOMEM;
            break;
        }
        if (BIO_read_filename(in, ifile) <= 0) {
            CRYPT_LOGERR("BIO_read_filename");
            ret = EPERM;
            break;
        }

        if (!(pk = PEM_read_bio_PrivateKey(in, NULL, NULL, pass))) {
            CRYPT_LOGERR("PEM_read_bio_PrivateKey");
            ret = ENOKEY;
            break;
        }

        if (pk->type != EVP_PKEY_RSA) {
            printf("Private key type not RSA\n");
            ret = EKEYREJECTED;
            break;
        }

        pk_pri = pk;

    } while(0);

    if (in) {
        BIO_free(in);
    }

    if (ret) {
        if (pk) {
            EVP_PKEY_free(pk);
        }
    }

    return ret;
}

int rsakey::load_pubkey(const char *ifile)
{
    int ret = 0;
    EVP_PKEY *pk = NULL;
    BIO *in = NULL;

    do {
        in = BIO_new(BIO_s_file());
        if (!in) {
            CRYPT_LOGERR("BIO_new: failed to create BIO");
            ret = ENOMEM;
            break;
        }
        if (BIO_read_filename(in, ifile) <= 0) {
            CRYPT_LOGERR("BIO_read_filename");
            ret = EPERM;
            break;
        }

        if (!(pk = PEM_read_bio_PUBKEY(in, NULL, NULL, NULL))) {
            CRYPT_LOGERR("PEM_read_bio_PUBKEY");
            ret = ENOKEY;
            break;
        }
        if (pk->type != EVP_PKEY_RSA) {
            printf("Public key type not RSA\n");
            ret = EKEYREJECTED;
            break;
        }

        pk_pub = pk;
    } while(0);

    if (in) {
        BIO_free(in);
    }

    if (ret) {
        if (pk) {
            EVP_PKEY_free(pk);
        }
    }

    return ret;
}

int rsakey::enc_dec(unsigned char *in, size_t inlen, unsigned char **outp,
                    size_t *outl, int dec)
{
    int ret;
    unsigned char *out = NULL;
    size_t outlen;
    const int pad = RSA_PKCS1_OAEP_PADDING;

    EVP_PKEY_CTX *ctx = NULL;
    EVP_PKEY *pk = dec ? pk_pri : pk_pub;

    do {
        ret = EINVAL;
        *outp = NULL;
        *outl = 0;

        if ((ctx = EVP_PKEY_CTX_new(pk, NULL)) == NULL) {
            CRYPT_LOGERR("EVP_PKEY_CTX_new");
            break;
        }

        if (dec) {
            if (EVP_PKEY_decrypt_init(ctx) <= 0) {
                CRYPT_LOGERR("EVP_PKEY_encrypt_init");
                break;
            }
            if (EVP_PKEY_CTX_set_rsa_padding(ctx, pad) <= 0) {
                CRYPT_LOGERR("EVP_PKEY_CTX_set_rsa_padding");
                break;
            }
            if (EVP_PKEY_decrypt(ctx, NULL, &outlen, in, inlen) <= 0) {
                CRYPT_LOGERR("EVP_PKEY_decrypt(NULL)");
                break;
            }
            out = (unsigned char *)OPENSSL_malloc(outlen+8);

            if (EVP_PKEY_decrypt(ctx, out, &outlen, in, inlen) <= 0) {
                CRYPT_LOGERR("EVP_PKEY_decrypt");
                OPENSSL_free(out);
                break;
            }
        } else {
            if (EVP_PKEY_encrypt_init(ctx) <= 0) {
                CRYPT_LOGERR("EVP_PKEY_encrypt_init");
                break;
            }
            if (EVP_PKEY_CTX_set_rsa_padding(ctx, pad) <= 0) {
                CRYPT_LOGERR("EVP_PKEY_CTX_set_rsa_padding");
                break;
            }

            if (EVP_PKEY_encrypt(ctx, NULL, &outlen, in, inlen) <= 0) {
                CRYPT_LOGERR("EVP_PKEY_encrypt(NULL)");
                break;
            }
            out = (unsigned char *)malloc(outlen);
            if (EVP_PKEY_encrypt(ctx, out, &outlen, in, inlen) <= 0) {
                CRYPT_LOGERR("EVP_PKEY_encrypt");
                break;
            }
        }
        ret = 0;
        *outp = out;
        *outl = outlen;
    } while(0);

    if (ctx) EVP_PKEY_CTX_free(ctx);

    return ret;
}

/*
 * private key encrypt
 openssl rsautl -in passwd -out /tmp/enc -inkey key -encrypt -passin pass:123456
 * public key decrypt
 openssl rsautl -in /tmp/enc -out /tmp/dec -inkey key.pub -pubin -decrypt
 * public key encrypt
 openssl rsautl -in passwd -out /tmp/enc -inkey key.pub -encrypt -pubin
 * private key decrypt
 openssl rsautl -in /tmp/enc -out /tmp/dec -inkey key -decrypt -passin pass:123456
*/
