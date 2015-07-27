#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#include <sasl.h>

/* send/recv library for IMAP4 style literals.

   really not important; just one way of doing length coded strings */

int send_string(FILE *f, const char *s, int l)
{
    int al;

    al = fprintf(f, "{%d}\r\n", l);
    fwrite(s, 1, l, f);
    fflush(f);

    printf("S:{%d}\n", l);
    while (l--) {
	if (isprint((unsigned char) *s)) {
	    printf("%c", *s);
	} else {
	    printf("...");
	    break;
	    //printf("[%02x]", (unsigned char) *s);
	}
	s++;
    }
    printf("\n");

    return al;
}

int recv_string(FILE *f, char *buf, int buflen)
{
    int c;
    int len, l;
    char *s;
    
    c = fgetc(f);
    if (c != '{') return -1;

    /* read length */
    len = 0;
    c = fgetc(f);
    while (isdigit(c)) {
	len = len * 10 + (c - '0');
	c = fgetc(f);
    }
    if (c != '}') return -1;
    c = fgetc(f);
    if (c != '\r') return -1;
    c = fgetc(f);
    if (c != '\n') return -1;

    /* read string */
    if (buflen <= len) {
	fread(buf, buflen - 1, 1, f);
	buf[buflen - 1] = '\0';
	/* discard oversized string */
	len -= buflen - 1;
	while (len--) (void)fgetc(f);

	len = buflen - 1;
    } else {
	fread(buf, len, 1, f);
	buf[len] = '\0';
    }

    l = len;
    s = buf;
    printf("R:{%d}\n", len);
    while (l--) {
	if (isprint((unsigned char) *s)) {
	    printf("%c", *s);
	} else {
	    printf("...");
	    break;
	    //printf("[%02x]", (unsigned char) *s);
	}
	s++;
    }
    printf("\n");

    return len;
}

void saslerr(int why, const char *what, sasl_conn_t *conn)
{
  fprintf(stderr, "%s: %s\n", what, sasl_errstring(why, NULL, NULL));
  fprintf(stderr, "  --> %s\n", conn?sasl_errdetail(conn):"");
}

void saslfail(int why, const char *what, sasl_conn_t *conn)
{
    saslerr(why, what, conn);
    exit(-1);
}

