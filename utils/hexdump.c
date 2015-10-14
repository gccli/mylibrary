#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include "hexdump.h"

#ifdef __cpluplus
extern "C" {
#endif

static char line[128];
static char *dump_line(const u_char *data, int len, int offset)
{
    int i, offs;
    int gap;
    const u_char *ch;

    offs = sprintf(line, "%05d   ", offset);
    for(ch = data, i = 0; i < len; i++) {
        offs += sprintf(line+offs, "%02x ", *ch);
        ch++;
        if (i == 7) offs += sprintf(line+offs, " ");
    }
    // print space to handle line less than 8 bytes
    if (len < 8) offs += sprintf(line+offs, " ");

    // fill hex gap with spaces if not full line
    if (len < 16) {
        gap = 16 - len;
        for (i = 0; i < gap; i++)
            offs += sprintf(line+offs, "   ");
    }
    offs += sprintf(line+offs, "   ");

    for(ch = data, i = 0; i < len; i++) {
        if (isprint(*ch)) offs += sprintf(line+offs, "%c", *ch);
        else              offs += sprintf(line+offs, ".");
        ch++;
    }

    line[offs] = 0;
    return line;
}

static char *dump_line_ex(const u_char *data, int len)
{
    int i, offs = 0;
    const u_char *ch = (u_char *)data;

    for(ch = data, i = 0; i < len; i++) {
        offs += sprintf(line+offs, "%02x ", *ch);
        ch++;
        if (i == 7) offs += sprintf(line+offs, " ");
    }
    // print space to handle line less than 8 bytes
    if (len < 8) offs += sprintf(line+offs, " ");
    offs += sprintf(line+offs, "|");

    for(ch = data, i = 0; i < len; i++) {
        if (isprint(*ch)) offs += sprintf(line+offs, "%c", *ch);
        else              offs += sprintf(line+offs, ".");
        ch++;
    }
    offs += sprintf(line+offs, "|");

    line[offs] = 0;

    return line;
}

void hexdump(void *data, size_t n, size_t size, FILE* fp)
{
    size_t len_rem = n * size;
    size_t line_width = 16;  // number of bytes per line
    int line_len;
    int offset = 0;       // zero-based offset counter
    const u_char *ch = (u_char *)data;

    if (len_rem <= line_width) {
        fprintf(fp, "%s\n", dump_line(ch, len_rem, offset));
        fflush(fp);
        return;
    }

    for ( ;; ) {
        line_len = line_width % len_rem;
        fprintf(fp, "%s\n", dump_line(ch, line_len, offset));

        len_rem = len_rem - line_len;
        ch = ch + line_len;
        offset = offset + line_width;
        if (len_rem <= line_width) {
            fprintf(fp, "%s\n", dump_line(ch, len_rem, offset));
            break;
        }
    }
    fflush(fp);
}

char *hexdumpex(void *data, size_t n, size_t size, char *ptr)
{
    size_t len_rem = n * size;
    size_t line_width = 16;  // number of bytes per line
    int line_len;
    int offs = 0;
    const u_char *ch = (u_char *)data;

    if (len_rem <= line_width) {
        sprintf(ptr, "%s\n", dump_line_ex(ch, len_rem));
        return ptr;
    }

    for ( ;; ) {
        line_len = line_width % len_rem;
        offs += sprintf(ptr+offs, "%s\n", dump_line_ex(ch, line_len));

        len_rem = len_rem - line_len;
        ch = ch + line_len;
        //offset = offset + line_width;
        if (len_rem <= line_width) {
            offs += sprintf(ptr+offs, "%s\n", dump_line_ex(ch, len_rem));
            break;
        }
    }

    return ptr;
}

#ifdef __cpluplus
}
#endif

////////////////////////////////////////////////////////////////////////////////
#ifdef _UT_TEST
#include <stdlib.h>
int main(int argc, char *argv[])
{
    FILE *fp;
    char *fbuf, buffer[8292];
    long flen;
    int bytes[5] = {0x40414244, 0x61636869, 0x7a7c7d7e, 0xaabbccff, 0x01020364};

    memset(buffer, 0, sizeof(buffer));
    fp = fopen(argv[1], "rb");
    if (fp) {
        fseek(fp, 0L, SEEK_END);
        flen = ftell(fp);
        rewind(fp);

        fbuf = (char *)calloc(flen, 1);
        size_t len = fread(fbuf, 1, flen, fp);
        if (len != flen) {
            printf("read error\n");
            return 1;
        }
        hexdump(fbuf, len, 1, stdout);
    }

    printf("\nbytes string: %s\n", hexdumpex(bytes, 5, sizeof(int), buffer));

    return 0;
}

#endif
