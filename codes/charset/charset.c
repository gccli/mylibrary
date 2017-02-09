#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iconv.h>
#include <wchar.h>

#include <errno.h>
#include <ctype.h>
#include <locale.h>

/**
 * Concept
 * Unicode
 *   Unicode defines a codespace of 1,114,112 code points in the range 0 to 10FFFF
 * UTF - Unicode Transformation Format
 * UTF-8 uses one to four bytes per code point and, being compact for Latin scripts and ASCII-compatible, provides the de facto standard encoding for interchange of Unicode text.

 * UTF-16 encodings specify the Unicode Byte Order Mark (BOM) for use at the beginnings of text files,

 * BOM - Unicode Byte Order Mark
 * plain - In the Unicode standard, a plane is a continuous group of 65536 code points.
 *
 *
 */

void print_hex_string(void *data, size_t len)
{
    size_t i;
    unsigned char *pstr, ch;

    pstr = (unsigned char *) data;
    for (i=0; i<len; ++i) {
        printf("%02x ", pstr[i]);
    }
    printf("\n");
    for (i=0; i<len; ++i) {
        ch = (char ) pstr[i];
        printf("%-2c ", isprint(ch) ? ch : '.');
    }
    printf("\n");
}

size_t utf8_encode(const char *text, size_t len, void *buffer, size_t buflen,
                   const char *encoding)
{
    size_t n, inlen, outlen;
    char  *in, *out;

    iconv_t cd = iconv_open(encoding, "UTF-8");
    if (cd == (iconv_t) -1) {
        return (size_t) -1;
    }

    in = (char *)text;
    out = (char *)buffer;

    inlen = len;
    outlen = buflen;

    len = (size_t) -1;
    n = iconv(cd, &in, &inlen, &out, &outlen);
    if (n == (size_t) -1) {
        printf("Error in iconv %d: %s\n", errno, strerror(errno));
        goto done;
    } else if (n > 0) {
        printf("%zu characters are converted in a non-reversible way\n", n);
    }

    len = out - (char *) buffer;
    printf("HEX string for %s (length:%zu) after convert\n", encoding, len);
    print_hex_string((void *)buffer, len);

done:
    iconv_close(cd);
    return len;
}

int main(int argc, char *argv[])
{
    char       *buffer;
    size_t      len, buflen;
    const char *text;
    wchar_t    *wstr;
    size_t      i;

    text = "世界,好\xf0\xa3\x8e\xb4好xx\xe2\x8c\x98";

    iconv_t cd = iconv_open("UTF-16", "UTF-8");
    if (cd == (iconv_t) -1) {
        return 1;
    }

    len = strlen(text);
    printf("Original UTF-8 string is:%s, length:%zu\n", text, len);
    printf("HEX string for UTF-8: (length:%zu)\n", len);
    print_hex_string((void *)text, len);

    buflen = 1024;
    buffer = (char *)calloc(buflen + 1, 1);

    utf8_encode(text, len, buffer, buflen, "UTF-16");
    utf8_encode(text, len, buffer, buflen, "UTF-32");
    utf8_encode(text, len, buffer, buflen, "UNICODE");
    len = utf8_encode(text, len, buffer, buflen, "WCHAR_T");
    if (len == (size_t) -1) {
        return 1;
    }

    wstr = (wchar_t *) buffer;


    for (i=0; i<len/sizeof(wchar_t); ++i) {
        printf("%6x  %7d\n", wstr[i], wstr[i]);

    }

    return 0;
}
