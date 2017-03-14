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
 *
 * Unicode - Unicode defines a codespace of 1,114,112 code points in the range 0 to 10FFFF
 * UTF - Unicode Transformation Format
 * UTF-8 uses one to four bytes per code point and, being compact for Latin scripts and ASCII-compatible
 *    - Code points larger than 127 are represented by multi-byte sequences
 *    - The first byte indicates the number of bytes in the sequence
 *    - Self-synchronization: the high-order bits of every byte determine the type of byte
 *      single bytes (0xxxxxxx), leading bytes (11...xxx), and continuation bytes (10xxxxxx)
 * UTF-16 encodings specify the Unicode Byte Order Mark (BOM) for use at the beginnings of text files,
 * BOM - Unicode Byte Order Mark
 * plain - In the Unicode standard, a plane is a continuous group of 65536 code points.
 * script - In Unicode, a script is a collection of letters and other written signs used to
 *          represent textual information in one or more writing systems
 *
 */

void print_hex_string(void *data, size_t len)
{
    size_t i;
    unsigned char *pstr, ch;

    if (data == NULL) {
        return ;
    }

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

char *unicode2utf8(wchar_t code)
{
    static char byte[8];

    memset(byte, 0, sizeof(byte));
    if (code < 0x7f) {
        byte[0] = code;
    } else if (code < 0x07ff) {
        byte[0] = (0x1f & (code>>6)) | 0xC0;
        byte[1] = (0x3f & code) | 0x80;
    } else if (code < 0xffff) {
        byte[0] = (0x1f & (code>>12)) | 0xe0;
        byte[1] = (0x3f & (code>>6)) | 0x80;
        byte[2] = (0x3f & code) | 0x80;
    } else if (code < 0x10ffff) {
        byte[0] = 0xf0 | (0x1f & (code>>18));
        byte[1] = 0x80 | (0x3f & (code>>12));
        byte[2] = 0x80 | (0x3f & (code>>6));
        byte[3] = 0x80 | (0x3f & code);
    } else {
        return NULL;
    }

    return byte;
}

wchar_t *utf8decode(const char *s, size_t *count)
{
    size_t      i, len;
    wchar_t    *str;
    const char *p, *end;

    len = strlen(s);
    str = (wchar_t *) calloc(len+1, 4);
    end = s+len;

    *count = 0;

    for(i=0, p=s; p<end; ) {
        if ((*p & 0xf0) == 0xf0) {
            if (p+4 > end) {
                goto incomplete;
            }

            str[i++] = (0x07 & *p)<<18 | (0x3f & *(p+2))<<12 | (0x3f & *(p+2))<<6 | (0x3f & *(p+3));
            p += 4;
        } else if ((*p & 0xe0) == 0xe0) {
            if (p+3 > end) {
                goto incomplete;
            }

            str[i++] = (0x0f & *p)<<12 | (0x3f & *(p+1))<<6 | (0x3f & *(p+2));
            p += 3;
        } else if ((*p & 0xc0) == 0xc0) {
            if (p+2 > end) {
                goto incomplete;
            }

            str[i++] = (0x1f & *p)<<6 | (0x3f & *(p+1));
            p += 2;
        } else {
            str[i++] = *p++;
        }
    }

    *count = i;

    return str;


incomplete:
    printf("Incomplete multibyte sequence\n");
    return NULL;
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
        printf("Error in iconv(%s) %d: %s\n", encoding, errno, strerror(errno));
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


#ifdef _test

int main(int argc, char *argv[])
{
    char       *buffer;
    size_t      len, buflen;
    const char *text;
    wchar_t    *wstr;
    size_t      i;

    text = "中®中,好\xf0\xa3\x8e\xb4好x\xe2\x8c\x98z";

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
        printf("U+%04x  %s\n", wstr[i], unicode2utf8(wstr[i]));
    }
    printf("U+2602  %s\n", unicode2utf8(0x2602));
    printf("U+2605  %s\n", unicode2utf8(0x2605));

    size_t cnt;
    len = strlen(text);
    wstr = utf8decode(text, &cnt);
    print_hex_string((void *)wstr, cnt*sizeof(wchar_t));

    return 0;
}

#endif
