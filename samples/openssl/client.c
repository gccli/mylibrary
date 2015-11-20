#include <sys/types.h>
#include <sys/syscall.h>
#include <assert.h>
#include <getopt.h>

#include "sslcommon.h"

char *host = "localhost";
int   port = 3200;
int   flags;

int main(int argc, char *argv[])
{
    int ret, index = 0;
    static struct option long_options[] = {
        {0, 0, 0, 0}
    };
    const char* optlist = "ih:p:";
    while (1){
        int c = getopt_long(argc, argv, optlist, long_options, &index);
        if (c == EOF) break;
        switch (c) {
        case 'h':
            host = strdup(optarg);
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 'i':
            flags = 1;
            break;
        case 0:
            break;
        default:
            printf("usage: %s [-i] [-h host] [-p port]\n", argv[0]);
            exit(0);
        }
    }

    SSLinit();
    SSLseeding(1024, "/tmp/sending");

    SSL *ssl = NULL;
    BIO *conn = NULL;
    SSL_CTX *ctx = SSLnew_client_ctx("certs");

    conn = BIO_new_ssl_connect(ctx);
    BIO_get_ssl(conn, &ssl);
    if (ssl == NULL)
        return -1;

    char strport[8] = {0};
    sprintf(strport, "%d", port);

    BIO_set_conn_hostname(conn, host);
    BIO_set_conn_port(conn, strport);
    BIO_set_nbio(conn, 1);
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    do {
        if ((ret = BIO_do_connect(conn)) > 0)
            break;
    } while(BIO_should_retry(conn));
    if (ret <= 0) {
        fprintf(stderr, "BIO_do_connect\n");
        SSL_LOGERR("BIO_do_connect");
        return 1;
    }

    if (BIO_do_handshake(conn) <=0 ) {
        fprintf(stderr, "BIO_do_handshake\n");
        return 1;
    }

    int wlen, rlen;
    char sndbuf[10240] = {0};
    char rcvbuf[1024] = {0};

    rlen = BIO_nb_read(conn, rcvbuf, sizeof(rcvbuf));
    if (rlen > 0) {
        rcvbuf[rlen] = 0;
        fprintf(stderr, "S: %s\n", rcvbuf);
    }
    printf("SSL connection opened\n");

    while(1) {
        rlen = fread(sndbuf, 1, sizeof(sndbuf), stdin);
        if (rlen == 0) {
            break;
        }
        wlen = BIO_nb_write(conn, sndbuf, rlen);
        printf("C: %d bytes sent\n", wlen);
    }

    sleep(1);
    rlen = BIO_nb_read(conn, rcvbuf, sizeof(rcvbuf));
    if (rlen > 0) {
        rcvbuf[rlen] = 0;
        fprintf(stderr, "S: %s", rcvbuf);
    }

    BIO_free_all(conn);

    return 0;
}
