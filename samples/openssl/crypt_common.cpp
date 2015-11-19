#include "crypt_common.h"

void ui2buf(uint32_t n, unsigned char *p)
{
    *p++ = (n >> 24) & 0xFF;
    *p++ = (n >> 16) & 0xFF;
    *p++ = (n >> 8) & 0xFF;
    *p++ = n & 0xFF;
}

uint32_t buf2ui(const unsigned char *p)
{
    return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

void ul2buf(uint64_t n, unsigned char *p)
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

uint64_t buf2ul(const unsigned char *p)
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
