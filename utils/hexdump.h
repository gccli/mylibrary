#ifndef HEXDUMP_H__
#define HEXDUMP_H__

#include <stdio.h>

#ifdef __cpluplus
extern "C" {
#endif

/**
 * dump @n elements of data given by @data, each @size bytes long, to the
 * stream pointed to by @fp
 */
void hexdump(void *data, size_t n, size_t size, FILE* fp);


/**
 * dump @n elements of data given by @data, each @size bytes long, to the
 * string pointed to by @ptr
 * return the point given by @ptr
 */
char *hexdumpex(void *data, size_t n, size_t size, char *ptr);

#ifdef __cpluplus
}
#endif

#endif
