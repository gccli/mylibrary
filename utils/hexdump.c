#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <setjmp.h>
#include "hexdump.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
#define countof(a) (sizeof (a) / sizeof *(a))

#if __GNUC__
#define NOTUSED __attribute__((unused))
#define NORETURN __attribute__((noreturn))
#endif

#define NARG_(a, b, c, d, e, f, g, h, N,...) N
#define NARG(...) NARG_(__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define PASTE(x, y) x##y
#define XPASTE(x, y) PASTE(x, y)
static unsigned char toprint(unsigned char chr)
{
    return (chr > 0x1f && chr < 0x7f)? chr : '.';
}
static const char *tooctal(char buf[3], unsigned char chr)
{
    if (chr > 0x1f && chr < 0x7f) {
        buf[0] = chr;
        buf[1] = '\0';
    } else {
        switch (chr) {
            case '\0':
                buf[0] = '\\';
                buf[1] = '0';
                buf[2] = '\0';
                break;
            case '\a':
                buf[0] = '\\';
                buf[1] = 'a';
                buf[2] = '\0';
                break;
            case '\b':
                buf[0] = '\\';
                buf[1] = 'b';
                buf[2] = '\0';
                break;
            case '\f':
                buf[0] = '\\';
                buf[1] = 'f';
                buf[2] = '\0';
                break;
            case '\n':
                buf[0] = '\\';
                buf[1] = 'n';
                buf[2] = '\0';
                break;
            case '\r':
                buf[0] = '\\';
                buf[1] = 'r';
                buf[2] = '\0';
                break;
            case '\t':
                buf[0] = '\\';
                buf[1] = 't';
                buf[2] = '\0';
                break;
            case '\v':
                buf[0] = '\\';
                buf[1] = 'v';
                buf[2] = '\0';
                break;
            default:
                buf[0] = "01234567"[0x7 & (chr >> 6)];
                buf[1] = "01234567"[0x7 & (chr >> 3)];
                buf[2] = "01234567"[0x7 & (chr >> 0)];
                break;
        }
    }
    return buf;
}
static const char *toshort(char buf[3], unsigned char chr)
{
    static const char map[][3] = {
        [0x00] = "nul", [0x01] = "soh", [0x02] = "stx", [0x03] = "etx",
        [0x04] = "eot", [0x05] = "enq", [0x06] = "ack", [0x07] = "bel",
        [0x08] = "bs",  [0x09] = "ht",  [0x0a] = "lf",  [0x0b] = "vt",
        [0x0c] = "ff",  [0x0d] = "cr",  [0x0e] = "so",  [0x0f] = "si",
        [0x10] = "dle", [0x11] = "dc1", [0x12] = "dc2", [0x13] = "dc3",
        [0x14] = "dc4", [0x15] = "nak", [0x16] = "syn", [0x17] = "etb",
        [0x18] = "can", [0x19] = "em",  [0x1a] = "sub", [0x1b] = "esc",
        [0x1c] = "fs",  [0x1d] = "gs",  [0x1e] = "rs",  [0x1f] = "us",
        [0x7f] = "del",
    };
    if (chr <= 0x1f || chr == 0x7f) {
        memcpy(buf, map[chr], 3);
    }
    else if (chr < 0x7f) {
        buf[0] = chr;
        buf[1] = '\0';
    }
    else {
        buf[0] = "0123456789abcdef"[0x0f & (chr >> 4)];
        buf[1] = "0123456789abcdef"[0x0f & chr];
        buf[2] = '\0';
    }
    return buf;
}
static inline bool hxd_isspace(unsigned char chr, bool nlok)
{
    static const unsigned char space[] = {
        ['\t'] = 1, ['\n'] = 1, ['\v'] = 1, ['\r'] = 1, ['\f'] = 1, [' '] = 1,
    };
    return (chr < sizeof space && space[chr] && (nlok || chr != '\n'));
}
static inline unsigned char skipws(const unsigned char **fmt, bool nlok)
{
    while (hxd_isspace(**fmt, nlok))
        ++*fmt;
    return **fmt;
}
static inline int getint(const unsigned char **fmt)
{
    static const int limit = ((INT_MAX - (INT_MAX % 10) - 1) / 10);
    int i = -1;
    if (**fmt >= '0' && **fmt <= '9') {
        i = 0;
        do {
            i *= 10;
            i += **fmt - '0';
            ++*fmt;
        } while (**fmt >= '0' && **fmt <= '9' && i <= limit);
    }
    return i;
}

#define F_HASH  1
#define F_ZERO  2
#define F_MINUS 4
#define F_SPACE 8
#define F_PLUS 16
#define FC2(x, y) (((0xff & (y)) << 8) | (0xff & (x)))
#define FC1(x) (0xff & (x))
#define FC(...) XPASTE(FC, NARG(__VA_ARGS__))(__VA_ARGS__)
static inline int getcnv(int *flags, int *width, int *prec, int *bytes,
                         const unsigned char **fmt)
{
    int ch;
    *flags = 0;
    for (; (ch = **fmt); ++*fmt) {
        switch (ch) {
        case '#':
            *flags |= F_HASH;
            break;
        case '0':
            *flags |= F_ZERO;
            break;
        case '-':
            *flags |= F_MINUS;
            break;
        case ' ':
            *flags |= F_SPACE;
            break;
        case '+':
            *flags |= F_PLUS;
            break;
        default:
            goto width;
        }
    }
    width:
    *width = getint(fmt);
    *prec = (**fmt == '.')? (++*fmt, getint(fmt)) : -1;
    *bytes = 0;
    switch ((ch = **fmt)) {
    case '%':
        break;
    case 'c':
        *bytes = 1;
        break;
    case 'd': case 'i': case 'o': case 'u': case 'X': case 'x':
        *bytes = 4;
        break;
    case 's':
        if (*prec == -1)
            return 0;
        *bytes = *prec;
        break;
    case '_':
        switch (*++*fmt) {
        case 'a':
            switch (*++*fmt) {
            case 'd':
                ch = FC('_', 'd');
                break;
            case 'o':
                ch = FC('_', 'o');
                break;
            case 'x':
                ch = FC('_', 'x');
                break;
            default:
                return 0;
            }
            *bytes = 0;
            break;
        case 'A':
            switch (*++*fmt) {
            case 'd':
                ch = FC('_', 'D');
                break;
            case 'o':
                ch = FC('_', 'O');
                break;
            case 'x':
                ch = FC('_', 'X');
                break;
            default:
                return 0;
            }
            *bytes = 0;
            /* XXX: Not supported yet. */
            return 0;
            break;
        case 'c':
            ch = FC('_', 'c');
            *bytes = 1;
            break;
        case 'p':
            ch = FC('_', 'p');
            *bytes = 1;
            break;
        case 'u':
            ch = FC('_', 'u');
            *bytes = 1;
            break;
        default:
            return 0;
        }
        break;
    }
    ++*fmt;
    return ch;
}
enum vm_opcode
{
    OP_HALT,                     /* 0/0 */
    OP_NOOP,                     /* 0/0 */
    OP_TRAP,                     /* 0/0 */
    OP_PC,                       /* 0/1 | push program counter */
    OP_TRUE,                     /* 0/1 | push true */
    OP_FALSE,                    /* 0/1 | push false */
    OP_ZERO,                     /* 0/1 | push 0 */
    OP_ONE,                      /* 0/1 | push 1 */
    OP_TWO,                      /* 0/1 | push 2 */
    OP_I8,                       /* 0/1 | load 8-bit unsigned int from code */
    OP_I16,                      /* 0/1 | load 16-bit unsigned int from code */
    OP_I32,                      /* 0/1 | load 32-bit unsigned int from code */
    OP_NEG,                      /* 1/1 | arithmetic negative */
    OP_SUB,                      /* 2/1 | S(-2) - S(-1) */
    OP_ADD,                      /* 2/1 | S(-2) + S(-1) */
    OP_NOT,                      /* 1/1 | logical not */
    OP_OR,                       /* 2/1 | logical or */
    OP_LT,                       /* 2/1 | S(-2) < S(-1) */
    OP_POP,                      /* 1/0 | pop top of stack */
    OP_DUP,                      /* 1/2 | dup top of stack */
    OP_SWAP,                     /* 2/2 | swap values at top of stack */
    OP_READ,                     /* 1/1 | read bytes from input buffer */
    OP_COUNT,                    /* 0/1 | count of bytes in input buffer */
    OP_PUTC,                     /* 0/0 | copy char directly to output buffer */
    OP_CONV,                     /* 5/0 | write conversion to output buffer */
    OP_CHOP,                     /* 1/0 | chop trailing characters from output buffer */
    OP_PAD,                      /* 1/0 | emit padding space to output buffer */
    OP_JMP,                      /* 2/0 | conditional jump to address */
    OP_RESET,                    /* 0/0 | reset input buffer position */
    /*
     * Optimized conversions with predicates.
     */
    #define OK_IS0FIXED(F, W, P, N) \
        (((W) == (N) && ((F) == F_ZERO) && (P) <= 0) || \
        ((W) == -1 && (P) == (N)) || \
        ((W) == (P) && (W) == (N)))
    #define OK_2XBYTE(F, W, P) OK_IS0FIXED((F), (W), (P), 2)
    OP_2XBYTE,
    #define OK_PBYTE(F, W, P) ((W) <= 1 && (P) <= 1)
    OP_PBYTE,
    #define OK_7XADDR(F, W, P) OK_IS0FIXED((F), (W), (P), 7)
    OP_7XADDR,
    #define OK_8XADDR(F, W, P) OK_IS0FIXED((F), (W), (P), 8)
    OP_8XADDR,
};

struct vm_state
{
    jmp_buf trap;
    int flags;
    size_t blocksize;
    int64_t stack[8];
    int sp;
    unsigned char code[4096];
    int pc;
    struct {
        unsigned char *base, *p, *pe;
        size_t address;
        bool eof;
    } i;
    struct {
        unsigned char *base, *p, *pe;
    } o;
};


#define vm_enter(M) setjmp((M)->trap)

NORETURN static void vm_throw(struct vm_state *M, int error)
{
    longjmp(M->trap, error);
}
static inline unsigned char vm_getc(struct vm_state *M)
{
    return (M->i.p < M->i.pe)? *M->i.p++ : 0;
}
static void vm_putc(struct vm_state *M, unsigned char ch)
{
    unsigned char *tmp;
    size_t size, p;
    if (!(M->o.p < M->o.pe)) {
        size = MAX(M->o.pe - M->o.base, 64);
        p = M->o.p - M->o.base;
        if (~size < size)
            vm_throw(M, ENOMEM);
        size *= 2;
        if (!(tmp = realloc(M->o.base, size)))
            vm_throw(M, errno);
        M->o.base = tmp;
        M->o.p = &tmp[p];
        M->o.pe = &tmp[size];
    }
    *M->o.p++ = ch;
}

static void vm_putx(struct vm_state *M, unsigned char ch)
{
    vm_putc(M, "0123456789abcdef"[0x0f & (ch >> 4)]);
    vm_putc(M, "0123456789abcdef"[0x0f & (ch >> 0)]);
}
static inline size_t vm_address(struct vm_state *M)
{
    return M->i.address + (M->i.p - M->i.base);
}
static void vm_push(struct vm_state *M, int64_t v)
{
    M->stack[M->sp++] = v;
}
static int64_t vm_pop(struct vm_state *M)
{
    return M->stack[--M->sp];
}
NOTUSED static int64_t vm_peek(struct vm_state *M, int i)
{
    return (i < 0)? M->stack[M->sp + i] : M->stack[i];
}
static void vm_conv(struct vm_state *M, int flags, int width, int prec, int fc,
                    int64_t word)
{
    char fmt[32], *fp, buf[256], label[3];
    const char *s = NULL;
    int i, len;
    switch (fc) {
        case FC('_', 'c'):
            s = tooctal(label, word);
            prec = (prec > 0)? MIN(prec, 3) : 3;
            fc = 's';
            break;
        case FC('_', 'p'):
            word = toprint(word);
            fc = 'c';
            break;
        case FC('_', 'u'):
            s = toshort(label, word);
            prec = (prec > 0)? MIN(prec, 3) : 3;
            fc = 's';
            break;
        case FC('_', 'd'):
            word = M->i.address + (M->i.p - M->i.base);
            fc = 'd';
            break;
        case FC('_', 'o'):
            word = M->i.address + (M->i.p - M->i.base);
            fc = 'o';
            break;
        case FC('_', 'x'):
            word = M->i.address + (M->i.p - M->i.base);
            fc = 'x';
            break;
        case FC('_', 'D'):
            /* FALL THROUGH */
        case FC('_', 'O'):
            /* FALL THROUGH */
        case FC('_', 'X'):
            fc = 'x';
            vm_throw(M, HXD_ENOTSUPP);
            break;
        case FC('s'):
            s = (const char *)M->i.p;
            if (prec <= 0 || prec > M->i.pe - M->i.p)
                prec = M->i.pe - M->i.p;
            break;
        case FC('c'):
            /* FALL THROUGH */
        case FC('d'): case FC('i'): case FC('o'):
        case FC('u'): case FC('X'): case FC('x'):
            break;
        default:
            vm_throw(M, HXD_ENOTSUPP);
            break;
    }                            /* switch() */
    fp = fmt;
    *fp++ = '%';
    if (flags & F_HASH)
        *fp++ = '#';
    if (flags & F_ZERO)
        *fp++ = '0';
    if (flags & F_MINUS)
        *fp++ = '-';
    if (flags & F_PLUS)
        *fp++ = '+';
    *fp++ = '*';
    *fp++ = '.';
    *fp++ = '*';
    *fp++ = fc;
    *fp = '\0';
    switch (fc) {
        case 's':
            len = snprintf(buf, sizeof buf, fmt, MAX(width, 0), MAX(prec, 0), s);
            break;
        case 'u':
            len = snprintf(buf, sizeof buf, fmt, MAX(width, 0), prec, (unsigned)word);
            break;
        default:
            len = snprintf(buf, sizeof buf, fmt, MAX(width, 0), prec, (int)word);
            break;
    }
    if (-1 == len)
        vm_throw(M, errno);
    if (len >= (int)sizeof buf)
        vm_throw(M, ENOMEM);
    for (i = 0; i < len; i++)
        vm_putc(M, buf[i]);
}

#define NEXT ++M->pc; goto exec
static void vm_exec(struct vm_state *M)
{
    int64_t v;
exec:
    switch (M->code[M->pc]) {
    case OP_HALT:
        return ;
    case OP_NOOP:
        NEXT;
    case OP_TRAP:
        vm_throw(M, HXD_EOOPS);
        NEXT;
    case OP_PC:
        vm_push(M, M->pc);
        NEXT;
    case OP_TRUE:
        vm_push(M, 1);
        NEXT;
    case OP_FALSE:
        vm_push(M, 0);
        NEXT;
    case OP_ZERO:
        vm_push(M, 0);
        NEXT;
    case OP_ONE:
        vm_push(M, 1);
        NEXT;
    case OP_TWO:
        vm_push(M, 2);
        NEXT;
    case OP_I8:
        vm_push(M, M->code[++M->pc]);
        NEXT;
    case OP_I16:
        v = M->code[++M->pc] << 8;
        v |= M->code[++M->pc];
        vm_push(M, v);
        NEXT;
    case OP_I32:
        v = M->code[++M->pc] << 24;
        v |= M->code[++M->pc] << 16;
        v |= M->code[++M->pc] << 8;
        v |= M->code[++M->pc];
        vm_push(M, v);
        NEXT;
    case OP_NEG:
        vm_push(M, -vm_pop(M));
        NEXT;
    case OP_SUB:{
        int64_t b = vm_pop(M);
        int64_t a = vm_pop(M);
        vm_push(M, a - b);
        NEXT;
    }
    case OP_ADD: {
        int64_t b = vm_pop(M);
        int64_t a = vm_pop(M);
        vm_push(M, a + b);
        NEXT;
    }
    case OP_NOT:
        vm_push(M, !vm_pop(M));
        NEXT;
    case OP_OR: {
        int64_t b = vm_pop(M);
        int64_t a = vm_pop(M);
        vm_push(M, a || b);
        NEXT;
    }
    case OP_LT: {
        int64_t b = vm_pop(M);
        int64_t a = vm_pop(M);
        vm_push(M, a < b);
        NEXT;
    }
    case OP_POP:
        vm_pop(M);
        NEXT;
    case OP_DUP: {
        int64_t v = vm_pop(M);
        vm_push(M, v);
        vm_push(M, v);
        NEXT;
    }
    case OP_SWAP: {
        int64_t x = vm_pop(M);
        int64_t y = vm_pop(M);
        vm_push(M, x);
        vm_push(M, y);
        NEXT;
    }
    case OP_READ: {
        int64_t i, n, v;
        n = vm_pop(M);
        v = 0;
        if (M->flags & HXD_BIG_ENDIAN) {
            for (i = 0; i < n && M->i.p < M->i.pe; i++) {
                v <<= 8;
                v |= *M->i.p++;
            }
        } else {
            for (i = 0; i < n && M->i.p < M->i.pe; i++) {
                v |= *M->i.p++ << (8 * i);
            }
        }
        vm_push(M, v);
        NEXT;
    }
    case OP_COUNT:
        vm_push(M, M->i.pe - M->i.p);
        NEXT;
    case OP_PUTC:
        vm_putc(M, M->code[++M->pc]);
        NEXT;
    case OP_CONV: {
        int fc = vm_pop(M);
        int prec = vm_pop(M);
        int width = vm_pop(M);
        int flags = vm_pop(M);
        int64_t word = vm_pop(M);
        vm_conv(M, flags, width, prec, fc, word);
        NEXT;
    }
    case OP_CHOP:
        v = vm_pop(M);
        while (v > 0 && M->o.p > M->o.base) {
            --M->o.p;
            --v;
        }
        NEXT;
    case OP_PAD:
        v = vm_pop(M);
        while (v-- > 0)
            vm_putc(M, ' ');
        NEXT;
    case OP_JMP: {
        int64_t pc = vm_pop(M);
        if (vm_pop(M)) {
            M->pc = pc % countof(M->code);
            goto exec;
        }
        NEXT;
    }
    case OP_RESET:
        M->i.p = M->i.base;
        NEXT;
    case OP_2XBYTE:
        vm_putx(M, vm_getc(M));
        NEXT;
    case OP_PBYTE:
        vm_putc(M, toprint(vm_getc(M)));
        NEXT;
    case OP_7XADDR: {
        size_t addr = vm_address(M);
        vm_putc(M, "0123456789abcdef"[0x0f & (addr >> 24)]);
        vm_putx(M, (addr >> 16));
        vm_putx(M, (addr >> 8));
        vm_putx(M, (addr >> 0));
        NEXT;
    }
    case OP_8XADDR: {
        size_t addr = vm_address(M);
        vm_putx(M, (addr >> 24));
        vm_putx(M, (addr >> 16));
        vm_putx(M, (addr >> 8));
        vm_putx(M, (addr >> 0));
        NEXT;
    }
    }
}

static void emit_op(struct vm_state *M, unsigned char code)
{
    if (M->pc >= (int)sizeof M->code - 1)
        vm_throw(M, ENOMEM);
    M->code[M->pc++] = code;
}
static void emit_int(struct vm_state *M, int64_t i)
{
    bool isneg;
    if ((isneg = (i < 0)))
        i *= -1;
    if (i > ((1LL << 32) - 1)) {
        vm_throw(M, ERANGE);
    }
    else if (i > ((1LL << 16) - 1)) {
        emit_op(M, OP_I32);
        emit_op(M, 0xff & (i >> 24));
        emit_op(M, 0xff & (i >> 16));
        emit_op(M, 0xff & (i >> 8));
        emit_op(M, 0xff & (i >> 0));
    }
    else if (i > ((1LL << 8) - 1)) {
        emit_op(M, OP_I16);
        emit_op(M, 0xff & (i >> 8));
        emit_op(M, 0xff & (i >> 0));
    }
    else {
        switch (i) {
            case 0:
                emit_op(M, OP_ZERO);
                break;
            case 1:
                emit_op(M, OP_ONE);
                break;
            case 2:
                emit_op(M, OP_TWO);
                break;
            default:
                emit_op(M, OP_I8);
                emit_op(M, 0xff & i);
                break;
        }
    }
    if (isneg) {
        emit_op(M, OP_NEG);
    }
}
static void emit_putc(struct vm_state *M, unsigned char chr)
{
    emit_op(M, OP_PUTC);
    emit_op(M, chr);
}
static void emit_jmp(struct vm_state *M, int *from)
{
    *from = M->pc;
    emit_op(M, OP_TRAP);
    emit_op(M, OP_TRAP);
    emit_op(M, OP_TRAP);
    emit_op(M, OP_TRAP);
    emit_op(M, OP_TRAP);
    emit_op(M, OP_TRAP);
}
static void emit_link(struct vm_state *M, int from, int to)
{
    int pc = M->pc;
    M->pc = from;
    emit_op(M, OP_PC);
    if (to < from) {
        if (from - to > 65535)
            vm_throw(M, ERANGE);
        emit_op(M, OP_I16);
        M->code[M->pc++] = 0xff & ((from - to) >> 8);
        M->code[M->pc++] = 0xff & ((from - to) >> 0);
        emit_op(M, OP_SUB);
    }
    else {
        if (to - from > 65535)
            vm_throw(M, ERANGE);
        emit_op(M, OP_I16);
        M->code[M->pc++] = 0xff & ((to - from) >> 8);
        M->code[M->pc++] = 0xff & ((to - from) >> 0);
        emit_op(M, OP_ADD);
    }
    emit_op(M, OP_JMP);
    M->pc = pc;
}
static void emit_unit(struct vm_state *M, int loop, int limit, int flags,
                      size_t *blocksize, const unsigned char **fmt)
{
    bool quoted = 0, escaped = 0;
    int consumes = 0, chop = 0;
    int L1, L2, C1 = 0, from, ch;
    loop = (loop < 0)? 1 : loop;
    /* loop counter */
    emit_int(M, 0);
    /* top of loop */
    L1 = M->pc;
    emit_op(M, OP_DUP);          /* dup counter */
    emit_int(M, loop);           /* push loop count */
    emit_op(M, OP_SWAP);
    emit_op(M, OP_SUB);          /* loop - counter */
    emit_op(M, OP_NOT);
    if (flags & HXD_NOPADDING) {
        emit_op(M, OP_COUNT);
        C1 = M->pc;              /* patch destination for unit size */
        emit_op(M, OP_TRAP);
        emit_op(M, OP_TRAP);
        emit_op(M, OP_LT);
        emit_op(M, OP_OR);
    }
    emit_jmp(M, &L2);
    emit_int(M, 1);
    emit_op(M, OP_ADD);
    while ((ch = **fmt)) {
        switch (ch) {
        case '%': {
            int fc, flags, width, prec, bytes;
            if (escaped)
                goto copyout;
            ++*fmt;
            if (!(fc = getcnv(&flags, &width, &prec, &bytes, fmt)))
                vm_throw(M, HXD_EFORMAT);
            --*fmt;
            if (fc == '%') {
                ch = '%';
                goto copyout;
            }
            if (limit >= 0 && bytes > 0) {
                bytes = MIN(limit - consumes, bytes);
                if (!bytes)  /* FIXME: define better error */
                    vm_throw(M, HXD_EDRAINED);
            }
            consumes += bytes;
            {
                int J1, J2;
                if (bytes > 0) {
                    if (width > 0) {
                        emit_op(M, OP_COUNT);
                        emit_jmp(M, &J1);
                        emit_int(M, width);
                        emit_op(M, OP_PAD);
                        emit_op(M, OP_TRUE);
                        emit_jmp(M, &J2);
                        emit_link(M, J1, M->pc);
                    } else {
                        emit_op(M, OP_COUNT);
                        emit_op(M, OP_NOT);
                        emit_jmp(M, &J2);
                    }
                }
                if (fc == 'x' && OK_2XBYTE(flags, width, prec)) {
                    emit_op(M, OP_2XBYTE);
                } else if (fc == FC('_', 'p') && OK_PBYTE(flags, width, prec)) {
                    emit_op(M, OP_PBYTE);
                } else if (fc == FC('_', 'x') && OK_7XADDR(flags, width, prec)) {
                    emit_op(M, OP_7XADDR);
                } else if (fc == FC('_', 'x') && OK_8XADDR(flags, width, prec)) {
                    emit_op(M, OP_8XADDR);
                } else {
                    emit_int(M, (fc == 's')? 0 : bytes);
                    emit_op(M, OP_READ);
                    emit_int(M, flags);
                    emit_int(M, width);
                    emit_int(M, prec);
                    emit_int(M, fc);
                    emit_op(M, OP_CONV);
                }
                if (bytes > 0)
                    emit_link(M, J2, M->pc);
            }
            chop = 0;
            break;
        }
        case ' ': case '\t': case '\n':
            if (quoted || escaped)
                goto copyout;
            goto epilog;
        case '"':
            if (escaped)
                goto copyout;
            quoted = !quoted;
            break;
        case '\\':
            if (escaped)
                goto copyout;
            escaped = 1;
            break;
        case '0':
            if (escaped)
                ch = '\0';
            goto copyout;
        case 'a':
            if (escaped)
                ch = '\a';
            goto copyout;
        case 'b':
            if (escaped)
                ch = '\b';
            goto copyout;
        case 'f':
            if (escaped)
                ch = '\f';
            goto copyout;
        case 'n':
            if (escaped)
                ch = '\n';
            goto copyout;
        case 'r':
            if (escaped)
                ch = '\r';
            goto copyout;
        case 't':
            if (escaped)
                ch = '\t';
            goto copyout;
        case 'v':
            if (escaped)
                ch = '\v';
            goto copyout;
        default:
copyout:
            emit_putc(M, ch);
            escaped = 0;
            if (hxd_isspace(ch, 0)) {
                chop++;
            }
            else {
                chop = 0;
            }
        }
        ++*fmt;
    }
epilog:
    if (loop > 0 && consumes < limit) {
        emit_int(M, limit - consumes);
        emit_op(M, OP_READ);
        emit_op(M, OP_POP);
        consumes = limit;
    }
    if (flags & HXD_NOPADDING) {
        if (consumes > 255)
            vm_throw(M, ERANGE);
        /* patch in our unit size */
        M->code[C1 + 0] = OP_I8;
        M->code[C1 + 1] = consumes;
    }
    emit_op(M, OP_TRUE);
    emit_jmp(M, &from);
    emit_link(M, from, L1);
    emit_link(M, L2, M->pc);
    emit_op(M, OP_POP);          /* pop loop counter */
    if (loop > 1 && chop > 0) {
        emit_int(M, chop);
        emit_op(M, OP_CHOP);
    }
    *blocksize += (size_t)(consumes * loop);
    return /* void */;
}
struct hdx
{
    struct vm_state vm;
};

struct hdx *hxd_open(int *error)
{
    struct hdx *X;
    if (!(X = calloc(1, sizeof *X)))
        return NULL;
    return X;
}


void hxd_close(struct hdx *X)
{
    free(X->vm.i.base);
    free(X->vm.o.base);
    free(X);
}

void hxd_reset(struct hdx *X)
{
    X->vm.i.address = 0;
    X->vm.i.p = X->vm.i.base;
    X->vm.o.p = X->vm.o.base;
    X->vm.pc = 0;
}

void hxd_reset_all(struct hdx *X)
{
    free(X->vm.i.base);
    free(X->vm.o.base);
    memset(X, 0, sizeof(*X));
}

int hxd_compile(struct hdx *X, const char *_fmt, int flags)
{
    const unsigned char *fmt = (const unsigned char *)_fmt;
    unsigned char *tmp;
    int error;
    hxd_reset(X);
    if ((error = vm_enter(&X->vm)))
        goto error;
    X->vm.flags = flags;
    if (!HXD_BYTEORDER(X->vm.flags)) {
        union {
            int i; char c;
        } u = {0};
        u.c = 1;
        X->vm.flags |= (u.i & 0xff)? HXD_LITTLE_ENDIAN : HXD_BIG_ENDIAN;
    }
    while (skipws(&fmt, 1)) {
        int lc, loop, limit, flags;
        size_t blocksize = 0;
        flags = X->vm.flags;
        emit_op(&X->vm, OP_RESET);
        do {
            loop = getint(&fmt);
            if ('/' == skipws(&fmt, 0)) {
                fmt++;
                limit = getint(&fmt);
                if (*fmt == '?') {
                    flags |= HXD_NOPADDING;
                    fmt++;
                }
            }
            else {
                limit = -1;
            }
            skipws(&fmt, 0);
            emit_unit(&X->vm, loop, limit, flags, &blocksize, &fmt);
        } while ((lc = skipws(&fmt, 0)) && lc != '\n');
        if (blocksize > X->vm.blocksize)
            X->vm.blocksize = blocksize;
    }
    emit_op(&X->vm, OP_HALT);
    memset(&X->vm.code[X->vm.pc], OP_TRAP, sizeof X->vm.code - X->vm.pc);
    if (!(tmp = realloc(X->vm.i.base, X->vm.blocksize)))
        goto syerr;
    X->vm.i.base = tmp;
    X->vm.i.p = tmp;
    X->vm.i.pe = &tmp[X->vm.blocksize];
    return 0;
syerr:
    error = errno;
error:
    hxd_reset(X);
    memset(X->vm.code, 0, sizeof X->vm.code);
    return error;
}

int hxd_write(struct hdx *X, const void *src, size_t len)
{
    const unsigned char *p, *pe;
    size_t n;
    int error;
    if ((error = vm_enter(&X->vm)))
        goto error;
    if (X->vm.i.pe == X->vm.i.base)
        vm_throw(&X->vm, HXD_EOOPS);
    p = src;
    pe = p + len;
    while (p < pe) {
        n = MIN(pe - p, X->vm.i.pe - X->vm.i.p);
        memcpy(X->vm.i.p, p, n);
        X->vm.i.p += n;
        p += n;
        if (X->vm.i.p < X->vm.i.pe)
            break;
        X->vm.i.p = X->vm.i.base;
        X->vm.pc = 0;
        vm_exec(&X->vm);
        X->vm.i.p = X->vm.i.base;
        X->vm.i.address += X->vm.blocksize;
    }
    return 0;
error:
    return error;
}
int hxd_flush(struct hdx *X)
{
    unsigned char *pe;
    int error;
    if ((error = vm_enter(&X->vm)))
        goto error;
    if (X->vm.i.p > X->vm.i.base) {
        pe = X->vm.i.pe;
        X->vm.i.pe = X->vm.i.p;
        X->vm.i.p = X->vm.i.base;
        X->vm.pc = 0;
        vm_exec(&X->vm);
        X->vm.i.p = X->vm.i.base;
        X->vm.i.pe = pe;
    }
    return 0;
error:
    return error;
}
size_t hxd_read(struct hdx *X, void *dst, size_t lim)
{
    unsigned char *p, *pe, *op;
    size_t n;
    p = dst;
    pe = p + lim;
    op = X->vm.o.base;
    while (p < pe && op < X->vm.o.p) {
        n = MIN(pe - p, X->vm.o.p - op);
        memcpy(p, op, n);
        p += n;
        op += n;
    }
    n = X->vm.o.p - op;
    memmove(X->vm.o.base, op, n);
    X->vm.o.p = &X->vm.o.base[n];
    return p - (unsigned char *)dst;
}
const char *hxd_strerror(int error)
{
    static const char *txt[] = {
        [HXD_EFORMAT - HXD_EBASE] = "invalid format",
        [HXD_EDRAINED - HXD_EBASE] = "unit drains buffer",
        [HXD_ENOTSUPP - HXD_EBASE] = "unsupported conversion sequence",
        [HXD_EOOPS - HXD_EBASE] = "machine traps",
    };
    if (error >= 0)
        return strerror(error);
    if (error >= HXD_EBASE && error < HXD_ELAST) {
        error -= HXD_EBASE;
        if (error < (int)countof(txt) && txt[error])
            return txt[error];
    }
    return "unknown error (hexdump)";
}                                /* hxd_strerror() */

static struct hdx *hdx = NULL;
char *hexdump(const char *fmt, void *data, size_t size, char *str)
{
    int error;
    char buf[256];
    size_t len, soff, doff;
    if (hdx == NULL) {
        if (!(hdx = hxd_open(&error))) {
            printf("open error: %s", hxd_strerror(error));
            return NULL;
        }
    }
    hxd_reset_all(hdx);

    if ((error = hxd_compile(hdx, fmt, 0))) {
        printf("format(%s) error: %s", fmt, hxd_strerror(error));
        return NULL;
    }

    soff = 0;
    doff = 0;
    while(size > soff) {
        len = sizeof(buf) > size ? size : sizeof(buf);
        if ((error = hxd_write(hdx, data+soff, len))) {
            printf("write: %s", hxd_strerror(error));
            return NULL;
        }
        soff += len;

        while ((len = hxd_read(hdx, buf, sizeof(buf)))) {
            memcpy(str+doff, buf, len);
            doff += len;
        }
    }

    if ((error = hxd_flush(hdx))) {
        printf("flush: %s", hxd_strerror(error));
    }
    while ((len = hxd_read(hdx, buf, sizeof(buf)))) {
        memcpy(str+doff, buf, len);
        doff += len;
    }
    str[doff] = 0;

    return str;
}

#ifdef __cplusplus
}
#endif


#if HEXDUMP_MAIN
#include "utilfile.h"
int main(int argc, char **argv)
{
    size_t len;
    char buffer[4096] = {0};
    char *fmt;
    char *fbuf = get_file_buffer(argv[1], &len);

    printf("File buffer length: %zu\n", len);

    fmt = HEXDUMP_x;
    printf("%s\n", hexdump(fmt, fbuf, len, buffer));
    fmt = HEXDUMP_C;
    printf("%s\n", hexdump(fmt, fbuf, len, buffer));
    fmt = HEXDUMP_i;
    printf("%s\n", hexdump(fmt, fbuf, len, buffer));
    printf("%s\n", hexdump("/1 \"0x%02x,\"", fbuf, len, buffer));

    return 0;
}
#endif
