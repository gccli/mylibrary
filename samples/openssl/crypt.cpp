#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <openssl/evp.h>

#include "cipher.h"
#include "rsa.h"

#include <utiltime.h>
extern "C" {
#include "utilfile.h"
#include <hexdump.h>
}

static uint64_t magic_number = 0xfeeeeeeffeeeeeef;
typedef struct _crypt_ctx {
    cipher *ciph;
    rsakey *rsa;
} crypt_ctx_t;

crypt_ctx_t ctx = {NULL, NULL};

/**
 * RSA default passphrase, used for encrypt private key
 * when export and load
 */
static unsigned char rsa_pass[] = {
    0x7b,0xfe,0x9f,0x3d,0x9e,0xc6,0x06,0x7e,
    0x70,0x35,0xe9,0x6a,0x1b,0x6e,0x94,0xbe
};

void long2buff(uint64_t n, unsigned char *p)
{
    *p++ = (n >> 56) & 0xFF;
    *p++ = (n >> 48) & 0xFF;
    *p++ = (n >> 40) & 0xFF;
    *p++ = (n >> 32) & 0xFF;
    *p++ = (n >> 24) & 0xFF;
    *p++ = (n >> 16) & 0xFF;
    *p++ = (n >> 8) & 0xFF;
    *p++ = n & 0xFF;
}

uint64_t buff2long(const unsigned char *p)
{
    return  (((uint64_t)(*p)) << 56) |          \
        (((uint64_t)(*(p+1))) << 48) |          \
        (((uint64_t)(*(p+2))) << 40) |          \
        (((uint64_t)(*(p+3))) << 32) |          \
        (((uint64_t)(*(p+4))) << 24) |          \
        (((uint64_t)(*(p+5))) << 16) |          \
        (((uint64_t)(*(p+6))) << 8)  |          \
        ((uint64_t)(*(p+7)));
}

int create_ctx(const char *private_key)
{
    int ret;
    const char *pubkey = "/tmp/key.pub";

    if (ctx.ciph == NULL) {
        ctx.ciph = new cipher();
        if (!ctx.ciph) {
            return ENOMEM;
        }
    }

    if (ctx.rsa == NULL) {
        ctx.rsa = new rsakey();
        if (!ctx.rsa) {
            return ENOMEM;
        }
        if ((ret = ctx.rsa->load_key(private_key, rsa_pass))) {
            return ret;
        }

        if ((ret = ctx.rsa->export_key(pubkey))) {
            return ret;
        }
        if ((ret = ctx.rsa->load_pubkey(pubkey))) {
            return ret;
        }
    }

    return 0;
}

int encrypt(const char *ifile, const char *ofile)
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

        ret = ctx.rsa->enc_dec(secret, sizeof(secret), &pout, &outl);
        if (ret) break;

        // header length
        len = sizeof(magic_number) + 4 + outl;

        // construct header
        long2buff(magic_number, buffer);
        memcpy(buffer+sizeof(magic_number), &len, 2);
        memcpy(buffer+sizeof(magic_number)+4, pout, outl);
        free(pout);

        fwrite(buffer, 1, len, fpo);

        ret = ctx.ciph->enc_dec_file(secret, sizeof(secret), fpi, fpo);
        if (ret) break;
    } while(0);

    if (fpi) fclose(fpi);
    if (fpo) fclose(fpo);

    return ret;
}

int decrypt(const char *ifile, const char *ofile)
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
        if (buff2long(buffer) != magic_number) {
            printf("magic number mismatch\n");
            break;
        }

        fread(buffer, 1, 4, fpi);
        memcpy(&len, buffer, 2);
        len = len & 0xffff;
        len = len - sizeof(magic_number) - 4;

        fread(buffer, 1, len, fpi);

        ret = ctx.rsa->enc_dec(buffer, len,  &pout, &outl, 1);
        if (ret) break;
        if (outl != AES_KEY_LEN) {
            printf("key length mismatch\n");
            break;
        }

        ret = ctx.ciph->enc_dec_file(pout, outl, fpi, fpo, 1);
    } while(0);

    if (fpi) fclose(fpi);
    if (fpo) fclose(fpo);

    return ret;
}

int main(int argc, char *argv[])
{
    int ret, index;
    int dec = 0;
    static struct option long_options[] = {
        {0, 0, 0, 0}
    };

    const char* optlist = "d";
    while (1){
        int c = getopt_long(argc, argv, optlist, long_options, &index);
        if (c == EOF) break;
        switch (c) {
        case 'd':
            dec = 1;
            break;
        case 0:
            break;
        default:
            printf("usage: %s [-d] in out\n", argv[0]);
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

    if (create_ctx("key")) {
        printf("failed to create context\n");
        return 1;
    }
    if (dec) {
        ret = decrypt(argv[optind],argv[optind+1]);
    } else {
        ret = encrypt(argv[optind],argv[optind+1]);
    }

    return ret;
}
// Verify
// in=cipher.h; ./crypt $in /tmp/enc; ll $in /tmp/enc;./crypt /tmp/enc /tmp/dec -d; diff $in /tmp/dec
//
// decrypt file
// openssl bf -d -in iname -K hexstring -iv hexstring
// openssl enc -bf-ecb -nopad -d -in test1.enc -K hexstring -iv hexstring
