#include "crypt_common.h"

#include <openssl/rand.h>


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
