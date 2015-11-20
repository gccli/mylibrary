#ifndef HEXDUMP_H__
#define HEXDUMP_H__

#define HXD_EBASE -(('D' << 24) | ('U' << 16) | ('M' << 8) | 'P')
#define HXD_ERROR(error) ((error) >= XD_EBASE && (error) < XD_ELAST)

enum hxd_errors
{
    HXD_EFORMAT = HXD_EBASE,
    /* a compile-time error signaling an invalid format string, format
       unit, or conversion specification syntax */

    HXD_EDRAINED,
    /* a compile-time error signaling that preceding conversions have
       already drained the input block */

    HXD_ENOTSUPP,
    /* a compile-time error returned for valid but unsupported
       conversion specifications */

    HXD_EOOPS,
    /* something horrible happened */

    HXD_ELAST
};                               /* enum hxd_errors */

#define hxd_error_t int          /* for documentation purposes only */

const char *hxd_strerror(hxd_error_t);

/*
 * H E X D U M P  C O R E  I N T E R F A C E S
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

struct hdx;

struct hdx *hxd_open(hxd_error_t *);

void hxd_close(struct hdx *);

void hxd_reset(struct hdx *);

#define HXD_BYTEORDER(x)  (0x03 & (x))
#define HXD_NATIVE         0x00
#define HXD_NETWORK        HXD_BIG_ENDIAN
#define HXD_BIG_ENDIAN     0x01
#define HXD_LITTLE_ENDIAN  0x02
#define HXD_NOPADDING      0x04

hxd_error_t hxd_compile(struct hdx *, const char *, int);
hxd_error_t hxd_write(struct hdx *, const void *, size_t);
hxd_error_t hxd_flush(struct hdx *);
size_t hxd_read(struct hdx *, void *, size_t);

/*
 * H E X D U M P  C O M M O N  F O R M A T S
 *
 * Predefined formats for hexdump(1) -b, -c, -C, -d, -o, -x, and xxd(1) -i.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define HEXDUMP_b "\"%07.7_ax \" 16/1 \"%03o \" \"\\n\""
#define HEXDUMP_c "\"%07.7_ax \" 16/1 \"%3_c \" \"\\n\""
#define HEXDUMP_C "\"%08.8_ax  \" 8/1 \"%02x \" \"  \" 8/1 \"%02x \"\n" \
    "\"  |\" 16/1 \"%_p\" \"|\\n\""
#define HEXDUMP_d "\"%07.7_ax \" 8/2 \"  %05u \" \"\\n\""
#define HEXDUMP_o "\"%07.7_ao   \" 8/2 \" %06o \" \"\\n\""
#define HEXDUMP_x "\"%07.7_ax \" 8/2 \"   %04x \" \"\\n\""

#define HEXDUMP_i "\"  \" 12/1? \"0x%02x, \" \"\\n\""


#define HXD_1 "/1 \"%02x\""
char *hexdump(const char *fmt, void *data, size_t size, char *str);

#endif
