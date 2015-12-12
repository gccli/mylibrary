
#define CS_BUFFER_LEN 2048
#define SC_BUFFER_LEN 8192
#define SLEEP_US 50000


#include <stdarg.h>
#include <syslog.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>

#include "sslcommon.h"
#include <openssl/pem.h>
#include <openssl/crypto.h>

#include "crypt.h"

BIO     *server_acc = NULL;
SSL_CTX *server_ctx = NULL;
SSL_CTX *client_ctx = NULL;
struct crypt_ctx *enc_ctx;

typedef struct {
    // Server side
    BIO *ss_client;                // Server side per-connection client BIO

    // Client side
    SSL *cs_ssl;
    BIO *cs_bio;
} Conn;
Conn *conn=NULL;

void debug(char *format,...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    putc('\n', stderr);
    va_end(args);

}

void plog(int level, const char *format,...)
{
    char str[8192];
    va_list args;
    va_start(args, format);
    vsprintf(str, format, args);
    va_end(args);
    syslog(level, "%s\n", str);
    debug("LOG: %.256s", str);
}

int serverside_init(int port)
{
    char tmp[128];

    server_ctx=SSLnew_server_ctx("certs/server.pem","certs/server.key",NULL);
    sprintf(tmp, "%d", port);

    SSL *ssl = NULL;
    BIO *client;

    client = BIO_new_ssl(server_ctx, 0);
    BIO_get_ssl(client, &ssl);

    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    // acc is a source/sink BIO
    if ((server_acc = BIO_new_accept(tmp)) == NULL)
        return 1;
    BIO_set_bind_mode(server_acc, BIO_BIND_REUSEADDR);

    BIO_set_accept_bios(server_acc,client);

    // set up the accept BIO
    if (BIO_do_accept(server_acc) <= 0) {
        SSL_LOGERR("init accept");
        return 1;
    }

    return 0;
}

int clientside_connect(SSL_CTX *ctx, const char *host, int port, Conn *conn)
{
    char tmp[128];

    conn->cs_bio = BIO_new_ssl_connect(ctx);
    BIO_get_ssl(conn->cs_bio, &conn->cs_ssl);
    if (conn->cs_ssl == NULL)
        return -1;

    sprintf(tmp, "%d", port);

    BIO_set_conn_hostname(conn->cs_bio, host);
    BIO_set_conn_port(conn->cs_bio, tmp);
    //BIO_set_nbio(conn->cs_bio, 1);
    SSL_set_mode(conn->cs_ssl, SSL_MODE_AUTO_RETRY);

    if (BIO_do_connect(conn->cs_bio) <= 0) {
        SSL_LOGERR("connect");
        return 1;
    }

    if (BIO_do_handshake(conn->cs_bio) <=0 ) {
        SSL_LOGERR("handshake");
        return 1;
    }

    return 0;
}

void sighandler(int signum)
{
    switch (signum) {
	case SIGINT:
	case SIGTERM:
	    plog(LOG_NOTICE, "SIGNAL Interrupt/terminate");
	    // If it's possible to remove pid file, try it..
	    // It's not guaranteed to succeed, because of setuid
	    exit(0);
	default:;
    }
}


void *threadfunc(void *param)
{
    Conn *conn = (Conn *) param;
    int err;

    if (BIO_do_handshake(conn->ss_client) <= 0) {
        SSL_LOGERR("thread handshake");
        goto error;
    }

    // Encrypt data
    err = encrypt_s(enc_ctx, conn->ss_client, conn->cs_bio);
    if (err != 0) {
        printf("encrypt stream fail\n");
    }

    fprintf(stderr, "Connection closed\n");

error:
    BIO_free_all(conn->ss_client);
    BIO_free_all(conn->cs_bio);
    return NULL;
}

int conn_accept()
{
    int ret;
    Conn *conn;
    if (BIO_do_accept(server_acc) <= 0) {
        SSL_LOGERR("accept");
        return -1;
    }

    printf("client connected\n");

    conn = calloc(1, sizeof(Conn));
    conn->ss_client = BIO_pop(server_acc);

    ret = clientside_connect(client_ctx, "localhost", 3200, conn);
    if (ret != 0) {
        return ret;
    }

    pthread_t th;
    pthread_attr_t thattr;
    pthread_attr_init(&thattr);
    pthread_attr_setdetachstate(&thattr, PTHREAD_CREATE_DETACHED);
    pthread_create(&th, &thattr, threadfunc, conn);

    return 0;
}

int main(int argc, char **argv)
{
    int ret;
    signal(SIGINT, sighandler);
    signal(SIGTERM, sighandler);
    signal(SIGPIPE, SIG_IGN);

    SSLinit();

    crypt_rsa_genkey("key");
    ret = crypt_create(&enc_ctx, "key");
    if (ret != 0) {
        printf("failed to create context\n");
        return 1;
    }

    openlog("sslproxy", LOG_PID, LOG_DAEMON);
//    client_ctx = SSLnew_client_ctx("certs");
    client_ctx = SSLnew_client_ctx("/etc/ssl/certs");
    serverside_init(4000);

    while(1) {
        ret = conn_accept();
        if (ret != 0) break;
    }
    return 0;
}
