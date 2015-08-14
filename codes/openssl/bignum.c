#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>


#include <openssl/rand.h>
#include <openssl/bn.h>

#include <sys/types.h>
#include <sys/syscall.h>

#include <pthread.h>
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

static BIGNUM *fibonacci(unsigned long n)
{
    BIGNUM *fib = BN_new();
    if (n == 0) {
        BN_dec2bn(&fib, "0");
    }
    else if (n == 1) {
        BN_dec2bn(&fib, "1");
    }
    else {
        BIGNUM *x = NULL;
        BIGNUM *y = NULL;
        BN_dec2bn(&x, "0");
        BN_dec2bn(&y, "1");
        unsigned long i=2;
        for(; i<=n; i++) {
            BN_add(fib, x, y);
            BN_copy(x, y);
            BN_copy(y, fib);
        }

        BN_free(x);
        BN_free(y);
    }
    return fib;
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

static void *threadfunc(void *param)
{
    seeding(128);

    // big number test
    BIGNUM *x = NULL;
    BIGNUM *y = NULL;
    BIGNUM *z = NULL;
    BN_dec2bn(&x, "4294901760");
    BN_hex2bn(&y, "FFFF0000");

    assert(BN_cmp(x, y) == 0);

    z = fibonacci(50);
    assert(BN_cmp(x, z) < 0);
    BN_free(z);

    printf("fibonacci(0) = %s\n", BN_bn2dec(fibonacci(0)));
    printf("fibonacci(1) = %s\n", BN_bn2dec(fibonacci(1)));
    printf("fibonacci(2) = %s\n", BN_bn2dec(fibonacci(2)));
    printf("fibonacci(3) = %s\n", BN_bn2dec(fibonacci(3)));
    printf("fibonacci(4) = %s\n", BN_bn2dec(fibonacci(4)));
    printf("fibonacci(5) = %s\n", BN_bn2dec(fibonacci(5)));
    printf("fibonacci(6) = %s\n", BN_bn2dec(fibonacci(6)));
    printf("fibonacci(7) = %s\n", BN_bn2dec(fibonacci(7)));
    printf("fibonacci(50) = %s\n", BN_bn2dec(fibonacci(50)));
    printf("fibonacci(51) = %s\n", BN_bn2dec(fibonacci(51)));
    printf("fibonacci(52) = %s\n", BN_bn2dec(fibonacci(52)));
    printf("fibonacci(60) = %s\n", BN_bn2dec(fibonacci(60)));
    printf("fibonacci(500) = %s\n", BN_bn2dec(fibonacci(500)));
    printf("fibonacci(600) = %s\n", BN_bn2dec(fibonacci(600)));

    BN_CTX *ctx = BN_CTX_new();
    BIGNUM *d = BN_new();
    BIGNUM *r = BN_new();
    BN_div(d, r, fibonacci(501), fibonacci(500), ctx);
    printf("fibonacci(501) / fibonacci(500) = %s\n", BN_bn2dec(d));
    printf("fibonacci(501) %% fibonacci(500) = %s\n", BN_bn2dec(r));

    // prime number test
    prime_generate(1024, 1);

    return NULL;
}

int main(int argc, char *argv[])
{
    int i, len;
    BIGNUM *x, *y, *m, *r;
    long long ll;
    char *endptr=NULL;
    unsigned char output[16] = {0};

    SSL_load_error_strings();

    if (argc > 1) {
        x = NULL;
        r = BN_new();
        BN_dec2bn(&x, argv[1]);
        ll = strtoll(argv[1], &endptr, 10);
        printf("Original number is %s, covert to 8 bytes integer: %llx\n",
               argv[1], ll);

        printf("bytes  : %d, bits: %d\n", BN_num_bytes(x), BN_num_bits(x));
        printf("HEX    : %s\n", BN_bn2hex(x));

        len = BN_bn2mpi(x, output);
        printf("size   : %d\n", len);
        printf("HEX    : ");
        for(i=0; i<len; ++i) {
            printf("%.2X", output[i]);
        }
        printf("\n");

        y = BN_mpi2bn(output, len, r);
        if (y == NULL) {
            printf("error in covert:%ld\n", ERR_get_error());
            return 1;
        }

        printf("bytes  : %d, bits: %d\n", BN_num_bytes(y), BN_num_bits(y));
        printf("HEX    : %s\n", BN_bn2hex(y));
    }

    pthread_t      th;
    pthread_attr_t thattr;
    pthread_attr_init(&thattr);
    pthread_attr_setdetachstate(&thattr, PTHREAD_CREATE_DETACHED);
    pthread_create(&th, NULL, threadfunc, NULL);
    pthread_join(th, NULL);


    // x^y && x^y % m
    BN_CTX *ctx = BN_CTX_new();
    r = BN_new();

    x = y = m = NULL;

    BN_dec2bn(&x, "712");
    BN_dec2bn(&y, "729");
    BN_dec2bn(&m, "10000");
    if (BN_exp(r, x, y, ctx))
        printf("x^y = %s\n", BN_bn2dec(r));
    if (BN_mod_exp(r, x, y, m, ctx))
        printf("x^y %% m = %s\n", BN_bn2dec(r));

    return 0;
}
