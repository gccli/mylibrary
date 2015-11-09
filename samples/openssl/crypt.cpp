#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/evp.h>

#include "cipher.h"


extern "C" {
#include "hexdump.h"
}

static unsigned char key[32];
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

static EVP_CIPHER_CTX ctx;
int main(int argc, char *argv[])
{
    int encrypt = 1;
    int key_len = 8;

    char tmpstr[1024] = {0};
    const char *inf = NULL;
    const char *outf = NULL;

    static struct option long_options[] = {
        {0, 0, 0, 0}
    };
    int index = 0;
    const char* optlist = "i:o:dk:";
    while (1){
        int c = getopt_long(argc, argv, optlist, long_options, &index);
        if (c == EOF) break;
        switch (c) {
        case 'i':
            inf = strdup(optarg);
            break;
        case 'o':
            outf = strdup(optarg);
            break;
        case 'd':
            encrypt = 0;
            break;
        case 'k':
            key_len = strlen(optarg);
            memcpy(key, optarg, key_len);
            break;
        case 0:
            break;
        default:
            printf("usage: %s [-i in] [-o out]\n", argv[0]);
            exit(0);
        }
    }
    if (key[0] == 0) init_key();
    printf("key(%d): %s\n", key_len, hexdumpex(key, key_len, 1, tmpstr));

    crypt_init_module();
    crypt_init(&ctx, key, key_len, encrypt);
    if (encrypt) {
        crypt_encrypt(&ctx, inf, outf);
    } else {
        crypt_decrypt(&ctx, inf, outf);
    }
    crypt_destroy(&ctx);

    return 0;

}
