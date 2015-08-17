#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <rabinpoly.h>
#include <getopt.h>

#include <sys/types.h>
#include <sys/stat.h>

unsigned int window_size = 64;
size_t min_block_size = 1024;
size_t avg_block_size = 8192;
size_t max_block_size = 65536;

int file_fp(RabinPoly *rp, FILE *fp, uint64_t prime)
{
    int rc;

    rp_from_stream(rp, fp);
    for (;;) {
        rc = rp_block_next(rp);
        if (rc) {
            assert (rc == EOF);
            break;
        }
        printf("block_offset:-%2zu block_size:%2zu  fingerprint: %ld -> %lx\n",
               rp->block_streampos, rp->block_size,
               (long)rp->fingerprint, rp->fingerprint);
    }

    assert(feof(fp));

    return 0;
}

int buffer_fp(RabinPoly *rp, char *src, size_t len, uint64_t prime)
{
    int rc;
    rp->poly = prime;
    rp_from_buffer(rp, (unsigned char *)src, len);

    for (;;) {
        rc = rp_block_next(rp);
        if (rc) {
            assert (rc == EOF);
            break;
        }
        printf("block_offset:-%2zu block_size:%2zu  fingerprint: %ld -> %lx\n",
               rp->block_streampos, rp->block_size,
               (long)rp->fingerprint, rp->fingerprint);
    }

    return 0;
}

int main(int argc, char **argv)
{
    int c;
    size_t len;
    char *src;
    FILE *fp;
    uint64_t prime = 0xbfe6b8a5bf378d83LL;
    RabinPoly *rp = NULL;

    if (argc < 2) {
        printf("%s [ -wamx ] filename\n", argv[0]);
        return 1;
    }

    while ((c = getopt(argc, argv, "w:a:m:x:")) != EOF) {
        switch(c) {
        case 'w':
            window_size = atoi(optarg);
            break;
        case 'a':
            avg_block_size = atoi(optarg);
            break;
        case 'm':
            min_block_size = atoi(optarg);
            break;
        case 'x':
            max_block_size = atoi(optarg);
            break;

        default:
            return 1;
            break;
        }
    }

    struct stat st;
    stat(argv[optind], &st);

    printf("---- window_size:%d average:%zu min:%zu, max:%zu file:%s size:%zu----\n",
           window_size, avg_block_size,
           min_block_size, max_block_size,
           argv[optind], st.st_size);


    rp = rp_new(window_size, avg_block_size, min_block_size,
                max_block_size, 4*max_block_size);
    if (rp == NULL) {
        printf("failed to create handler\n");
        return 1;
    }

    fp = fopen(argv[optind], "r");
    if (!fp) {
        printf("failed to open '%s'\n", argv[optind]);
        return 0;
    }
    file_fp(rp, fp, prime);
    rp_free(rp);

    rp = rp_new(window_size, avg_block_size, min_block_size,
                max_block_size, 4*max_block_size);
    src = (char *)calloc(st.st_size+1, 1);
    rewind(fp);
    //fseek(fp, 0L, SEEK_SET);
    len = fread(src, 1, st.st_size, fp);
    buffer_fp(rp, src, len, prime);
    rp_free(rp);

    return 0;
}
