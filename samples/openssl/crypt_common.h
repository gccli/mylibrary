#ifndef CRYPT_COMMON_H__
#define CRYPT_COMMON_H__

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>

#define CRYPT_LOGERR(str) do {                                    \
        char tmp_errstr[256];                                     \
        unsigned long tmp_errno;                                  \
        while((tmp_errno = ERR_get_error())) {                    \
            printf("%s %s\n", str,                                \
                   ERR_error_string(tmp_errno, tmp_errstr));      \
	}                                                         \
    } while(0)

/**
 * Initialize the cryptography module for both Symmetric symmetric and asymmetric
 * Call this function before any crypt operation
 */
int crypt_init();
int crypt_destroy();

/**
 * Generate @len bytes random secret key specified by @key
 */
int crypt_gen_key(void *key, size_t len);




#endif
