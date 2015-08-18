#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <openssl/rand.h>
#include <openssl/bn.h>

#include <sys/types.h>
#include <sys/syscall.h>

#include <assert.h>
#include <getopt.h>

static void seeding(int size)
{
    /* Read size bytes from /dev/random and seed the PRNG with it */
    RAND_load_file("/dev/random", size);
    /* Write a seed file */
    RAND_write_file("prngseed.dat");
    /* Read the seed file in its entirety and print the number of bytes	obtained */
    int nb = RAND_load_file("prngseed.dat", -1);
    printf("Seeded the PRNG with %d byte(s) of data from prngseed.dat.\n", nb);
}

static BN_CTX *bn_ctx = NULL;

static bool is_prime(uint64_t n, BIGNUM *x)
{
    if (!bn_ctx)
        bn_ctx = BN_CTX_new();


    if (BN_is_prime_ex(x, BN_prime_checks, bn_ctx, NULL))
        return true;

    return false;
}


int prime_gen_status(int code, int j, BN_GENCB *cb)
{
    char *str;

    if (code == 0) {
        str = BN_bn2hex(cb->arg);
        printf("Found potential prime #%d %s (%s)\n", j, str,
            is_prime(0, (BIGNUM *)cb->arg)?"Yes":"No");
        if (str) OPENSSL_free(str);
    }
    else if (code == 1) {
        printf(".");
    }
    else {
        printf("Got one!\n");
    }

    return 1;
}

BIGNUM *prime_generate_ex(int bits)
{
    int ret;
    char *str;
    BIGNUM *prime = BN_new();

    BN_GENCB gencb;
    BN_GENCB_set(&gencb, prime_gen_status, prime);

    ret = BN_generate_prime_ex(prime, bits, false, NULL, NULL, &gencb);
    if (!ret) {
        printf("failed to generate prime\n");
        return NULL;
    }
    str = BN_bn2hex(prime);
    if (str) {
        printf("\nFound prime: %s\n", str);
        OPENSSL_free(str);
    }

    return prime;
}


void get_all_primes(uint64_t from, uint64_t to)
{
    uint64_t i, range,count=0;

    if (from >= to)  {
        printf("Bad range\n");
        return ;
    }

    BIGNUM *prime = BN_new();
    if (from % 2 == 0) from += 1;
    range = (to - from)/2;
    printf("Test all prime from %lu   -   %lu\n", from, to);

    for(i=from; i<to-2; i+=2) {
        BN_set_word(prime, i);
        count++;
        if (is_prime(0, prime)) {
            printf("prime: %lu 0x%lX %.2f%%\n", i, i, 100.*count/range);
        }
    }

}


int main(int argc, char *argv[])
{
    BIGNUM *x;
    uint64_t val;
    char *str, *endptr = NULL;
    get_all_primes(0xbfe6b8a5bf378d83, 0xbfe6b8a5bf378d83+10000);
    //get_all_primes(0x3, 0xff);

    SSL_load_error_strings();
    seeding(128);
    if (argc > 1) {
        str = argv[1];
        if (strncmp(str, "0x", 2) == 0 || strncmp(str, "0X", 2) == 0)
            str += 2;

        x = NULL;
        BN_hex2bn(&x, argv[1]);
        val = strtoll(argv[1], &endptr, 10);

        BN_hex2bn(&x, str);
        val = (uint64_t)strtoq(argv[1], &endptr, 0);

        if (errno != 0)
            perror("strtoll");

        if ((errno == ERANGE && val == UINT64_MAX)||(errno != 0 && val == 0)) {
            printf("can not covert to long, %s\n", strerror(errno));
            val = 0;
        }
        if (endptr == argv[1]) {
            printf("No digits were found\n");
            val = 0;
        }
        if (val != 0) {
            printf("Original number is %s, covert to 8 bytes integer: %p\n",
                argv[1], (void *)val);
        }

        printf("bytes    : %d, bits: %d\n", BN_num_bytes(x), BN_num_bits(x));
        printf("HEX      : %s\n", BN_bn2hex(x));
        printf("Is Prime : %s\n", is_prime(val, x)?"Yes":"No");

    }

    prime_generate_ex(64);

    return 0;
}
