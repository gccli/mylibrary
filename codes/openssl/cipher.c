#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <openssl/rand.h>
#include <openssl/bn.h>
#include <openssl/evp.h>

#include <sys/types.h>
#include <sys/syscall.h>

#include <pthread.h>
#include <assert.h>
#include <getopt.h>

static unsigned char key[128];
static unsigned char iv[128];

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


void select_random_key(unsigned char *key, int len)
{
    int i;
    RAND_bytes(key, len);

    for(i=0; i<len; ++i)
        printf("%02x", key[i]);
    printf("\n");
}


void select_random_iv(unsigned char *iv, int len)
{
    RAND_pseudo_bytes(iv, len);
}


static void fencrpty(const char *filename, char **outp, int *outl)
{
    EVP_CIPHER_CTX ctx;
    EVP_EncryptInit(&ctx, EVP_bf_cbc(), key, NULL);

    FILE* fp = fopen(filename, "rb");
    if (fp == NULL)
        return ;

    char buffer[1024];
    char *outbuf;
    int length, temp=0, offset=0, outlen=0;
    fseek(fp, 0L, SEEK_END);
    length = ftell(fp);
    rewind(fp);                  // fseek(stream, 0L, SEEK_SET)

    outbuf = calloc(1, length+sizeof(buffer));
    for(; offset < length ; ) {
        int len = fread(buffer, 1, sizeof(buffer), fp);
        if (len > 0) {
            offset += len;
            EVP_EncryptUpdate(&ctx, (unsigned char *) &outbuf[outlen], &temp, (unsigned char *) buffer, len);
            outlen += temp;
        }
        else if (len <= 0) break;
    }
    EVP_EncryptFinal(&ctx, (unsigned char *) &outbuf[outlen], &temp);
    outlen += temp;

    *outp = outbuf;
    *outl = outlen;

    fclose(fp);
}


static void fdecrpty(const char *filename, char **outp, int *outl)
{
    EVP_CIPHER_CTX ctx;
    EVP_DecryptInit(&ctx, EVP_bf_cbc(), key, NULL);

    FILE* fp = fopen(filename, "rb");
    if (fp == NULL)
        return ;

    char buffer[1024];
    char *outbuf;
    int length, temp=0, offset=0, outlen=0;
    fseek(fp, 0L, SEEK_END);
    length = ftell(fp);
    rewind(fp);                  // fseek(stream, 0L, SEEK_SET)

    outbuf = calloc(1, length+sizeof(buffer));
    for(; offset < length ; ) {
        int len = fread(buffer, 1, sizeof(buffer), fp);
        if (len > 0) {
            offset += len;
            EVP_DecryptUpdate(&ctx, (unsigned char *) &outbuf[outlen], &temp, (unsigned char *) buffer, len);
            outlen += temp;
        }
        else if (len <= 0) break;
    }
    EVP_DecryptFinal(&ctx, (unsigned char *) &outbuf[outlen], &temp);
    outlen += temp;

    *outp = outbuf;
    *outl = outlen;

    fclose(fp);
}


int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("usage: %s filename\n", argv[0]);
        return 0;
    }
    const char *encfile = "cipher.bin";
    const char *decfile = "plain.txt";

    seeding(128);

    printf("EVP_MAX_KEY_LENGTH:%d\n", EVP_MAX_KEY_LENGTH);
    select_random_key(key, EVP_MAX_KEY_LENGTH);
    select_random_iv(iv, EVP_MAX_IV_LENGTH);

    memset(key, 0, sizeof(key));
    strcpy(key, "lijing");

    char *enc = NULL, *dec = NULL;
    int len = 0;
    FILE *fp;

    printf("encrypt file '%s' to '%s' and decrypt it to '%s'\ndiff %s %s\n",
        argv[1], encfile, decfile, argv[1], decfile);
    fencrpty(argv[1], &enc, &len);
    if ((fp = fopen(encfile, "wb"))) {
        fwrite(enc, 1, len, fp);
        fclose(fp); fp = NULL;
    }
    free(enc);

    len = 0;
    fdecrpty(encfile, &dec, &len);
    if ((fp = fopen(decfile, "wb"))) {
        fwrite(dec, 1, len, fp);
        fclose(fp); fp = NULL;
    }
    free(dec);

    return 0;
}
