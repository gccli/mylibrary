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

static bool is_prime(uint64_t n, BIGNUM *x)
{
    BN_CTX *ctx = NULL;
    ctx = BN_CTX_new();


    if (BN_is_prime_ex(x, BN_prime_checks, ctx, NULL))
        return true;

    return false;
}


static const char *kill_line = "\033[0G\033[0K";
static char line[128];
static void prime_status(int code, int arg, void *cb_arg)
{
    if (code == 0) {
        sprintf(line, "Found potential prime #%d ", arg+1);
        write(fileno(stderr), kill_line, strlen(kill_line));
        write(fileno(stderr), line, strlen(line));
    }
    else if (code == 1 && arg && !(arg % 10)) {
        printf(".");
    }
    else  {
        printf("Got one!\n");
    }
}

// safe prime: a prime p so that (p-1)/2 is also prime
BIGNUM *prime_generate(int bits, int safe)
{
    char *str;
    BIGNUM *prime;
    printf("\nSearching for a %sprime %d bits in size ...", (safe ? "safe " : ""), bits);
    prime = BN_generate_prime(NULL, bits, safe, NULL, NULL, prime_status, NULL);
    if (!prime)
        return NULL;
    str = BN_bn2dec(prime);
    if (str)
    {
        printf("\nFound prime: %s\n", str);
        OPENSSL_free(str);
    }

    return prime;
}

int main(int argc, char *argv[])
{
    BIGNUM *x;
    uint64_t val;
    char *endptr=NULL;

    SSL_load_error_strings();
    seeding(128);
    if (argc > 1) {
        x = NULL;
        BN_dec2bn(&x, argv[1]);
        val = strtoll(argv[1], &endptr, 10);

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

    prime_generate(1024, 0);

    return 0;
}
