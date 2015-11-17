#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include "cipher.h"
#include "rsa.h"

#include <hexdump.h>
#include <utiltime.h>
#include <utilfile.h>

int verbose = 0;
const char *pri_key = "key";
const char *pub_key = "key.pub";


static unsigned char secret[AES_KEY_LEN];
static EVP_PKEY *rsa_prikey;
static EVP_PKEY *rsa_pubkey;

/*
static unsigned char rsa_pass[] = {
    0x7b,0xfe,0x9f,0x3d,0x9e,0xc6,0x06,0x7e,
    0x70,0x35,0xe9,0x6a,0x1b,0x6e,0x94,0xbe
    };*/

static unsigned char rsa_pass[] = {
    '1','2','3','4','5','6'
};

EVP_PKEY *rsa_gen_key()
{
    int type = EVP_PKEY_RSA;
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

/**
 * Load RSA private key
 */
EVP_PKEY *rsa_load_key(const char *ifile, unsigned char *pass)
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

/**
 * Load RSA public key
 */
EVP_PKEY *rsa_load_pubkey(const char *ifile)
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

/**
 * Export RSA public key to file @pubkey
 */
int export_pubkey(EVP_PKEY *pk, const char *pubkey)
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

int export_key(EVP_PKEY *pk, const char *prikey, unsigned char *pass)
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

int rsa_dec_enc(EVP_PKEY *pk, int enc, unsigned char *in, size_t inlen,
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

        if (enc) {
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
        } else {
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
    int ret = 0;
    if (access(pri_key, R_OK) == 0) {
        rsa_prikey = rsa_load_key(pri_key, rsa_pass);
        rsa_pubkey = rsa_load_pubkey(pub_key);
    } else {
        rsa_prikey = rsa_gen_key();
        if (rsa_prikey) {
            ret = export_key(rsa_prikey, pri_key, rsa_pass);
            if (ret == 0) {
                export_pubkey(rsa_prikey, pub_key);
                rsa_pubkey = rsa_load_pubkey(pub_key);
            }
        }
    }

    if (verbose && rsa_prikey && rsa_pubkey) {
        RSA_print_fp(stdout, EVP_PKEY_get1_RSA(rsa_prikey), 0);
        RSA_print_fp(stdout, EVP_PKEY_get1_RSA(rsa_pubkey), 0);
    }

    return ret;
}

int main(int argc, char *argv[])
{
    static EVP_PKEY *pk;
    EVP_CIPHER_CTX ctx;

    FILE *fp = NULL, *out = NULL;
    const char *keyfile = "passwd";
    const char *algo = "aes-256-cbc";
    int ret;
    int index = 0;
    int decrypt = 0;
    int use_pubkey = 0;

    unsigned char buffer[512], *pout;
    size_t len, outlen;

    static struct option long_options[] = {
        {0, 0, 0, 0}
    };

    const char* optlist = "vdp";
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
            printf("usage: %s [-vdp ] in out\n", argv[0]);
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
    if (optind < 1) {
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
        len = len & 0xffff;
        fread(buffer, 1, len-4, fp);
        ret = rsa_dec_enc(pk, 0, buffer, len-4, &pout, &outlen);
        assert(outlen == sizeof(secret));
        assert(memcmp(pout, secret, outlen) == 0);
        free(pout);

        // write encrypt file
        ret = crypt_init_ex(&ctx, secret, sizeof(secret), algo, decrypt);
        assert(ret == 0);

        ret = dec_enc_f2b(&ctx, fp, (char **)&pout, &outlen);
        assert(ret == 0);
        fwrite(pout, 1, outlen, out);
        free(pout);

        printf("%s: decrypted file %s -> %s\n", use_pubkey?"PUB":"PRI",
               argv[optind], argv[optind+1]);
    } else {
        ret = rsa_dec_enc(pk, 1, secret, sizeof(secret), &pout, &outlen);
        assert(ret == 0);

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
        ret = dec_enc_f2b(&ctx, fp, (char **)&pout, &outlen);
        assert(ret == 0);
        fwrite(pout, 1, outlen, out);
        free(pout);
        printf("%s: encrypted file %s -> %s\n", use_pubkey?"PUB":"PRI",
               argv[optind], argv[optind+1]);
    }

    crypt_destroy(&ctx);
    fclose(fp);
    fclose(out);

    return 0;
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
