#ifndef BASE64_H__
#define BASE64_H__

#define MAX_BASE64_ENCODED_SIZE(size) ((size) / 3 * 4 + 2+2)
#define MAX_BASE64_DECODED_SIZE(size) ((size) / 4 * 3 + 3)


char *base64_encode(const void *src, size_t src_size, int *dst_size);
char *base64_decode(const void *src, size_t src_size, int *dst_size);

#endif 
