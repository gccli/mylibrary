#include <sys/types.h>
#include <sys/syscall.h>
#include <assert.h>
#include <getopt.h>

#include <openssl/bio.h>

#include "sslcommon.h"

char *host = "localhost";
int   port = 3200;
int   flags;

int main(int argc, char *argv[])
{
    int ret, index = 0;
    static struct option long_options[] = {
        {"verify-return-error", 0, 0, 0},
        {"verify-depth",        1, 0, 0},
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

    SSL *ssl = NULL;
    BIO *conn = NULL, *errbio = NULL;
    char tmp[256] = {0};
    SSL_CTX *ctx = NULL;

    ctx = SSLnew_client_ctx("/etc/ssl/certs");
    errbio = SSLgetstderr();
    conn = BIO_new_ssl_connect(ctx);
    BIO_get_ssl(conn, &ssl);
    if (ssl == NULL)
        return -1;

    if (!SSL_set_tlsext_host_name(ssl, host)) {
        BIO_printf(errbio, "Unable to set TLS servername extension.\n");
        SSL_print_err("SSL_set_tlsext_host_name");
        return -1;
    }

    sprintf(tmp, "%d", port);
    BIO_set_conn_hostname(conn, host);
    BIO_set_conn_port(conn, tmp);
    BIO_set_nbio(conn, 1);
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    do {
        if ((ret = BIO_do_connect(conn)) > 0)
            break;
    } while(BIO_should_retry(conn));
    if (ret <= 0) {
        SSL_print_err("BIO_do_connect");
        return 1;
    }

    if (BIO_do_handshake(conn) <=0 ) {
        SSL_print_err("BIO_do_handshake");
        return 1;
    }

    int wlen, rlen;
    char sndbuf[10240] = {0};
    char rcvbuf[1024] = {0};

    rlen = BIO_nb_read(conn, rcvbuf, sizeof(rcvbuf));
    if (rlen > 0) {
        rcvbuf[rlen] = 0;
        BIO_printf(errbio, "S: %s\n", rcvbuf);
    }

    while(1) {
        rlen = fread(sndbuf, 1, sizeof(sndbuf), stdin);
        if (rlen == 0) {
            break;
        }
        wlen = BIO_nb_write(conn, sndbuf, rlen);
        BIO_printf(errbio, "C: %d bytes sent\n", wlen);
    }

    sleep(1);
    rlen = BIO_nb_read(conn, rcvbuf, sizeof(rcvbuf));
    if (rlen > 0) {
        rcvbuf[rlen] = 0;
        BIO_printf(errbio, "S: %s", rcvbuf);
    }

    BIO_free_all(conn);

    return 0;
}
