#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/evp.h>

#include "cipher.h"
extern "C" {
#include "hexdump.h"
#include "utiltime.h"
#include "utilfile.h"
}

static const char    *algo = "DES3";
static EVP_CIPHER_CTX ctx;
static unsigned char  key[32];
static unsigned int   key_len = 8;

static void init_key()
{
    FILE *fp;
    const char *keyfile = "passwd";
    if ((fp = fopen(keyfile, "rb")) == NULL) {
        fp = fopen("/dev/random", "rb");
        if (fp) {
            fread(key, 1, sizeof(key), fp);
            fclose(fp);
            fp = fopen(keyfile, "wb");
            fwrite(key, 1, sizeof(key), fp);
            fclose(fp);
        }
    } else {
        fread(key, 1, sizeof(key), fp);
        fclose(fp);
    }
}

static const char *cipher_mode(int mode)
{
    switch(mode) {
    case EVP_CIPH_STREAM_CIPHER:
        return "stream";
    case EVP_CIPH_ECB_MODE:
        return "ECB";
    case EVP_CIPH_CBC_MODE:
        return "CBC";
    case EVP_CIPH_CFB_MODE:
        return "CFB";
    case EVP_CIPH_OFB_MODE:
        return "OFB";
    default:
        return "Unknown";
    }
}

static void dump_cipher_ctx(EVP_CIPHER_CTX *ctx)
{
    int i, mode, offs;
    char tmpstr[128] = {0};

    if (EVP_CIPHER_CTX_iv_length(ctx) > 0) {
        for(i=0,offs=0; i<EVP_CIPHER_CTX_iv_length(ctx); ++i)
            offs += sprintf(tmpstr+offs, "%.2x", ctx->iv[i]);
        printf("  iv: %s(%d)", tmpstr, EVP_CIPHER_CTX_iv_length(ctx));
    }
    mode = EVP_CIPHER_CTX_mode(ctx);
    printf("  mode: %d(%s), block-size:%d\n", mode, cipher_mode(mode),
           EVP_CIPHER_CTX_block_size(ctx));
    printf("  key(%d): %s", key_len, hexdumpex(key, key_len, 1, tmpstr));
}

int main(int argc, char *argv[])
{
    int index = 0;
    int flags = 0;
    double start;
    static struct option long_options[] = {
        {0, 0, 0, 0}
    };

    const char* optlist = "dk:c:";
    while (1){
        int c = getopt_long(argc, argv, optlist, long_options, &index);
        if (c == EOF) break;
        switch (c) {
        case 'd':
            flags |= FLAG_DECRYPT;
            break;
        case 'k':
            key_len = strlen(optarg);
            memcpy(key, optarg, key_len);
            break;
        case 'c':
            algo = strdup(optarg);
            break;
        case 0:
            break;
        default:
            printf("usage: %s [-i in] [-o out]\n", argv[0]);
            exit(0);
        }
    }
    if (key[0] == 0) init_key();
    crypt_init_module();

    if (getenv("cipher")) {
        algo = getenv("cipher");
    }

    printf("%s \"%s\" with '%s' algorithm, write to \"%s\":\n",
           (flags & FLAG_DECRYPT) ?"Decrypt":"Encrypt", argv[optind], algo,
           argv[optind+1]);
    printf("  size: %s\n", file_size(argv[optind]));

    if (crypt_init_ex(&ctx, key, key_len, algo, flags)) {
        return 1;
    }
    dump_cipher_ctx(&ctx);
    start = timing_start();
    crypt_encrypt_file(&ctx, argv[optind], argv[optind+1]);
    printf("  timecost: %.3f\n", timing_cost(start));

    crypt_destroy(&ctx);
    return 0;
}
// Verify
// in=cipher.h; ./crypt $in /tmp/enc; ll $in /tmp/enc;./crypt /tmp/enc /tmp/dec -d; diff $in /tmp/dec
//
// decrypt file
// openssl bf -d -in iname -K hexstring -iv hexstring
// openssl enc -bf-ecb -nopad -d -in test1.enc -K hexstring -iv hexstring
