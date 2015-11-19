#ifndef CRYPT_COMMON_H__
#define CRYPT_COMMON_H__

#include <stdint.h>

/**
 * Utilis convert between 32/64 bits integer and buffer (big-endian)
 */
void ui2buf(uint32_t n, unsigned char *p);
void ul2buf(uint64_t n, unsigned char *p);
uint32_t buf2ui(const unsigned char *p);
uint64_t buf2ul(const unsigned char *p);

#endif
