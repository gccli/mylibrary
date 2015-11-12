#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include "cipher.h"


#include <hexdump.h>
#include <utiltime.h>
#include <utilfile.h>

int verbose = 0;
const char *pri_key = "key";
const char *pub_key = "key.pub";
static char pass[] = "123456";

static unsigned char secret[AES_KEY_LEN];
static EVP_PKEY *rsa_prikey;
static EVP_PKEY *rsa_pubkey;

EVP_PKEY *gen_key(int type = EVP_PKEY_RSA)
{
    EVP_PKEY_CTX *ctx;
    EVP_PKEY *pkey = NULL;

    do {
        ctx = EVP_PKEY_CTX_new_id(type, NULL);
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

        if (EVP_PKEY_keygen(ctx, &pkey) <= 0)
            CRYPT_LOGERR("EVP_PKEY_keygen");
    } while(0);


    return pkey;
}

EVP_PKEY *load_key(const char *ifile, char *pass)
{
    int ret = 0;
    EVP_PKEY *pk = NULL;
    BIO *in = NULL;

    do {
        in = BIO_new(BIO_s_file());
        BIO_read_filename(in, ifile);

        if (!(pk = PEM_read_bio_PrivateKey(in, NULL, NULL, pass))) {
            CRYPT_LOGERR("PEM_read_bio_PrivateKey");
            ret = ENOKEY;
            break;
        }
        if (pk->type != EVP_PKEY_RSA) {
            printf("Private key type not RSA\n");
            ret = EKEYREJECTED;
        }

    } while(0);

    if (in) {
        BIO_free(in);
    }

    if (ret) {
        if (pk) {
            EVP_PKEY_free(pk);
            pk = NULL;
        }
    }

    return pk;
}

EVP_PKEY *load_pubkey(const char *ifile, char *pass)
{
    int ret = 0;
    EVP_PKEY *pk = NULL;
    BIO *in = NULL;

    do {
        in = BIO_new(BIO_s_file());
        BIO_read_filename(in, ifile);

        if (!(pk = PEM_read_bio_PUBKEY(in, NULL, NULL, pass))) {
            CRYPT_LOGERR("PEM_read_bio_PUBKEY");
            ret = ENOKEY;
            break;
        }
        if (pk->type != EVP_PKEY_RSA) {
            printf("Public key type not RSA\n");
            ret = EKEYREJECTED;
        }

    } while(0);

    if (in) {
        BIO_free(in);
    }

    if (ret) {
        if (pk) {
            EVP_PKEY_free(pk);
            pk = NULL;
        }
    }

    return pk;
}

int export_key(EVP_PKEY *pk, char *prikey, char *pubkey, char *pass)
{
    int ret, len;
    BIO *out;
    const EVP_CIPHER *cipher;

    // export private key
    out = NULL;
    out = BIO_new(BIO_s_file());
    assert(1 == BIO_write_filename(out, prikey));
    if (pass) len = strlen(pass);
    if (len > 0) {
        cipher = EVP_aes_128_cbc();
    } else {
        cipher = NULL;
        pass = NULL;
    }
    ret = PEM_write_bio_PrivateKey(out, pk, cipher, NULL, 0, NULL, pass);
    if (ret <= 0) {
        CRYPT_LOGERR("PEM_write_bio_PrivateKey");
    }
    BIO_free(out);

    // export public key
    out = NULL;
    out = BIO_new(BIO_s_file());
    assert(1 == BIO_write_filename(out, pubkey));

    RSA *rsa = EVP_PKEY_get1_RSA(pk);
    if (PEM_write_bio_RSA_PUBKEY(out, rsa) <= 0) {
        CRYPT_LOGERR("PEM_write_bio_RSA_PUBKEY");
    }

    return 0;
}

int decrypt_encrypt(EVP_PKEY *pk, int enc, unsigned char *in, size_t inlen,
                    unsigned char **outp, size_t *outl)
{
    int ret;

    EVP_PKEY_CTX *ctx = NULL;
    unsigned char *out = NULL;
    size_t outlen;
    int pad = RSA_PKCS1_OAEP_PADDING;

    do {
        ret = EINVAL;
        *outp = NULL;
        *outl = 0;

        if ((ctx = EVP_PKEY_CTX_new(pk, NULL)) == NULL) {
            CRYPT_LOGERR("EVP_PKEY_CTX_new");
            break;
        }
        if (EVP_PKEY_encrypt_init(ctx) <= 0) {
            CRYPT_LOGERR("EVP_PKEY_encrypt_init");
            break;
        }
        if (EVP_PKEY_CTX_set_rsa_padding(ctx, pad) <= 0) {
            CRYPT_LOGERR("EVP_PKEY_CTX_set_rsa_padding");
            break;
        }

        // Determine buffer length
        if (enc) {
            if (EVP_PKEY_decrypt(ctx, NULL, &outlen, in, inlen) <= 0) {
                CRYPT_LOGERR("EVP_PKEY_CTX_decrypt(NULL)");
                break;
            }
            out = (unsigned char *)malloc(outlen);
            if (EVP_PKEY_decrypt(ctx, out, &outlen, in, inlen) <= 0) {
                CRYPT_LOGERR("EVP_PKEY_CTX_decrypt");
                break;
            }
        } else {
            if (EVP_PKEY_decrypt(ctx, NULL, &outlen, in, inlen) <= 0) {
                CRYPT_LOGERR("EVP_PKEY_CTX_decrypt(NULL)");
                break;
            }
            out = (unsigned char *)malloc(outlen);
            if (EVP_PKEY_decrypt(ctx, out, &outlen, in, inlen) <= 0) {
                CRYPT_LOGERR("EVP_PKEY_CTX_decrypt");
                OPENSSL_free(out);
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

static int get_rsa_key()
{
    if (access(pri_key, R_OK) == 0) {
        rsa_prikey = load_key(pri_key, pass);
        rsa_pubkey = load_pubkey(pub_key, NULL);
        if (verbose) {
            RSA_print_fp(stdout, EVP_PKEY_get1_RSA(rsa_pubkey), 0);
        }

    } else {
        rsa_prikey = gen_key();
        if (rsa_prikey) {
            export_key(rsa_prikey, (char *)pri_key, (char *)pub_key, pass);
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    static EVP_PKEY *pk;
    EVP_CIPHER_CTX ctx;

    FILE *fp = NULL, *out = NULL;
    const char *keyfile = "passwd";
    const char *algo = "aes_256_cbc";
    int ret;
    int index = 0;
    int decrypt = 0;
    int use_pubkey = 0;

    unsigned char buffer[256], *pout;
    size_t len, outlen;

    static struct option long_options[] = {
        {0, 0, 0, 0}
    };

    const char* optlist = "vdpc:";
    while (1){
        int c = getopt_long(argc, argv, optlist, long_options, &index);
        if (c == EOF) break;
        switch (c) {
        case 'v':
            verbose++;
            break;
        case 'd':
            decrypt = FLAG_DECRYPT;
            break;
        case 'p':
            use_pubkey = 1;
            break;
        case 0:
            break;
        default:
            printf("usage: %s [-i in] [-o out]\n", argv[0]);
            exit(0);
        }
    }

    // Init openssl library
    crypt_init_module();

    // Init or load secret key
    if ((fp = fopen(keyfile, "rb")) == NULL) {
        if ((fp = fopen(keyfile, "wb"))) {
            crypt_gen_key(secret, sizeof(secret));
            fwrite(secret, 1, sizeof(secret), fp);
        }
    } else {
        fread(secret, 1, sizeof(secret), fp);
    }
    if (fp) fclose(fp);

    // Generate RSA key pair if not exists
    get_rsa_key();

    // encrypt/decrypt file
    if (optind < 2) {
        printf("%s [-vpd] in out\n", argv[0]);
        return EINVAL;
    }
    fp = fopen(argv[optind], "rb");
    assert(fp != NULL);
    out = fopen(argv[optind+1], "wb");
    assert(out != NULL);

    pk = use_pubkey ? rsa_pubkey : rsa_prikey;
    if (decrypt) {
        fread(buffer, 1, 4, fp);
        memcpy(&len, buffer, 2);
        fread(buffer, 1, len, fp);
        ret = decrypt_encrypt(pk, 0, buffer, len, &pout, &outlen);
        if (memcmp(pout, secret, outlen) != 0) {
            printf("pass not match\n");
            return EINVAL;
        }
        free(pout);

        // write encrypt file
        ret = crypt_init_ex(&ctx, secret, sizeof(secret), algo, decrypt);
        assert(ret == 0);
        ret = dec_enc_file_to_buffer(&ctx, fp, (char **)&pout,
                                     (int *)&outlen);
        assert(ret == 0);
        fwrite(pout, 1, outlen, out);
        free(pout);
    } else {
        ret = decrypt_encrypt(pk, 1, secret, sizeof(secret), &pout, &outlen);
        assert(ret == 0);
        printf("outlen %zu\n", outlen);

        // write file header
        len = outlen + 4;                // header length
        *(unsigned short *)buffer = len;
        crypt_gen_key(buffer+2, 2);      // random reserved bytes
        memcpy(buffer+4, pout, outlen);
        free(pout);
        fwrite(buffer, 1, len, out);

        // write encrypt file
        ret = crypt_init_ex(&ctx, secret, sizeof(secret), algo, decrypt);
        assert(ret == 0);
        ret = dec_enc_file_to_buffer(&ctx, fp, (char **)&pout,
                                     (int *)&outlen);
        assert(ret == 0);
        fwrite(pout, 1, outlen, out);
        free(pout);
    }

    fclose(fp);
    fclose(out);

    return 0;
}
