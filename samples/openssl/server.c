#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/syscall.h>
#include <assert.h>
#include <getopt.h>
#include <signal.h>

#include "sslcommon.h"
#include "utilfile.h"

int port;
void *threadfunc(void *param)
{
    BIO *client = (BIO *) param;
    if (BIO_do_handshake(client) <= 0) {
        SSL_LOGERR("thread handshake");
        BIO_free_all(client);
        return NULL;
    }
    int sock = 0;
    BIO_get_fd(client, &sock);

    struct sockaddr_in peer;
    socklen_t solen = sizeof(peer);
    memset(&peer, 0, solen);
    getpeername(sock, (struct sockaddr *) &peer, &solen);
    fprintf(stderr, "Connection opened from %s:%d\n",
            inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));

    char tmp[128] = {0};
    char str[256] = {0};
    int fd = get_tmpfile(tmp);
    sprintf(str, "OK Server<%ld> greeting, write to:%s\r\n", thread_id(), tmp);
    BIO_puts(client, str);
    printf("%s", str);

    int  err, total = 0;
    char buffer[819200];
    do
    {
        if ((err = BIO_read(client, buffer, sizeof(buffer))) <= 0) {
            if (err == -1) {
                SSL_LOGERR("BIO_read");
            }
            break ;
        }
        BIO_write(client, "OK\r\n", 4);
        write(fd, buffer, err);
        total += err;
    } while(err > 0);

    BIO_free_all(client);

    fprintf(stderr, "Connection closed, %d bytes received.\n", total);

    return NULL;
}

int main(int argc, char *argv[])
{
    static struct option long_options[] = {
        {0, 0, 0, 0}
    };
    int index = 0;
    const char* optlist = "p:";
    while (1){
        int c = getopt_long(argc, argv, optlist, long_options, &index);
        if (c == EOF) break;
        switch (c) {
        case 'p':
            port = atoi(optarg);
            break;
        case 0:
            break;
        default:
            printf("usage: %s [-h host] [-p port]\n", argv[0]);
            exit(0);
        }
    }

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGPIPE);
    pthread_sigmask (SIG_BLOCK, &mask, NULL);

    SSLinit();
    SSLseeding(1024, "/tmp/sending");

    SSL_CTX *ctx = SSLnew_server_ctx("certs/server.pem", "lijing");

    char strport[8];
    sprintf(strport, "%d", port);

    SSL *ssl = NULL;
    BIO *acc, *client;

    // create a filter SSL BIO
    client = BIO_new_ssl(ctx, 0);
    BIO_get_ssl(client, &ssl);
    if (ssl == NULL)
        return 1;
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    // acc is a source/sink BIO
    if ((acc = BIO_new_accept(strport)) == NULL)
        return 1;
    BIO_set_bind_mode(acc, BIO_BIND_REUSEADDR);

    // when a new connection is acceptede on 'acc'
    // the 'client' will be 'dupilcated' and have the new socket BIO push into it.
    BIO_set_accept_bios(acc,client);

    // set up the accept BIO
    if (BIO_do_accept(acc) <= 0) {
        SSL_LOGERR("init accept");
        return 1;
    }

    printf("Server started, waiting for connection\n");
    for(;;)
    {
        // wait for incoming connection
        if (BIO_do_accept(acc) <= 0) {
            SSL_LOGERR("accept");
            break;
        }
        client = BIO_pop(acc);

        pthread_t th;
        pthread_attr_t thattr;
        pthread_attr_init(&thattr);
        pthread_attr_setdetachstate(&thattr, PTHREAD_CREATE_DETACHED);
        pthread_create(&th, &thattr, threadfunc, client);
    }

    BIO_free(acc);
    THREAD_cleanup();

    fprintf(stderr,"done\n");

    return 0;
}
