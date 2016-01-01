#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

static int url_isunreserved(unsigned char in)
{
  switch (in) {
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    case 'a': case 'b': case 'c': case 'd': case 'e':
    case 'f': case 'g': case 'h': case 'i': case 'j':
    case 'k': case 'l': case 'm': case 'n': case 'o':
    case 'p': case 'q': case 'r': case 's': case 't':
    case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
    case 'A': case 'B': case 'C': case 'D': case 'E':
    case 'F': case 'G': case 'H': case 'I': case 'J':
    case 'K': case 'L': case 'M': case 'N': case 'O':
    case 'P': case 'Q': case 'R': case 'S': case 'T':
    case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
    case '-': case '.': case '_': case '~':
      return 1;
    default:
      break;
  }
  return 0;
}

char *url_escape(const char *string, size_t length)
{
    size_t alloc = (length ? length : strlen(string))+1;
    char *ns;
    char *testing_ptr = NULL;
    unsigned char in; /* we need to treat the characters unsigned */
    size_t newlen = alloc;
    size_t strindex=0;
    size_t len;

    ns = malloc(alloc);
    if(!ns)
        return NULL;

    len = alloc-1;
    while(len--) {
        in = *string;

        if(url_isunreserved(in))
            /* just copy this */
            ns[strindex++]=in;
        else {
            /* encode it */
            newlen += 2; /* the size grows with two, since this'll become a %XX */
            if(newlen > alloc) {
                alloc *= 2;
                testing_ptr = realloc(ns, alloc);
                if(!testing_ptr) {
                    free( ns );
                    return NULL;
                }
                else {
                    ns = testing_ptr;
                }
            }

            snprintf(&ns[strindex], 4, "%%%02X", in);
            strindex+=3;
        }
        string++;
    }
    ns[strindex]=0; /* terminate it */
    return ns;
}

int url_unescape(const char *string, size_t length,
                 char **ostring, size_t *olen)
{
    size_t alloc = (length?length:strlen(string))+1;
    char *ns = malloc(alloc);
    unsigned char in;
    size_t strindex=0;
    unsigned long hex;

    if(!ns)
        return ENOMEM;

    while(--alloc > 0) {
        in = *string;
        if(('%' == in) && (alloc > 2) && isxdigit(string[1]) && isxdigit(string[2])) {
            /* this is two hexadecimal digits following a '%' */
            char hexstr[3];
            char *ptr;
            hexstr[0] = string[1];
            hexstr[1] = string[2];
            hexstr[2] = 0;

            hex = strtoul(hexstr, &ptr, 16);
            in = hex & 0xff;
            string+=2;
            alloc-=2;
        }

        ns[strindex++] = in;
        string++;
    }
    ns[strindex] = 0;

    if(olen)
        *olen = strindex;
    *ostring = ns;

    return 0;
}


#ifdef _TEST
#include <unistd.h>
#include "utilfile.h"
#include "utilsha1.h"
int main(int argc, char *argv[])
{
    int fd;
    char name[64], *buf, *out = NULL, *verify;
    size_t len, outlen;
    buf = get_file_buffer(argv[1], &len);
    if (buf) {
        if (argc > 2) {
            if (url_unescape(buf, len, &out, &outlen) == 0) {
                printf("%.*s\n", (int)outlen, out);
                if ((verify = url_escape(out, outlen))) {
                    fd = get_tmpfile(name, 0666, NULL, NULL);
                    if (fd > 0) {
                        printf("verify file:%s\n", name);
                        write(fd, verify, strlen(verify));
                        close(fd);
                    }
                }
            }
        } else {
            if ((out = url_escape(buf, len)))
                printf("%s\n", out);
        }
        free(buf);
        if (out) free(out);
    }
}
#endif
