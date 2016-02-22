#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/syscall.h>
#include <assert.h>
#include <getopt.h>
#include <signal.h>

#include <utils/debug.h>
#include <utils/file.h>

#include "sslcommon.h"

static int port = 3200; // default port

// verify
// openssl s_client -CApath /etc/ssl/certs/ -connect localhost:3200
void *threadfunc(void *param)
{
    BIO *client = (BIO *) param;
    if (BIO_do_handshake(client) <= 0) {
        SSL_print_err("thread handshake");
        BIO_free_all(client);
        return NULL;
    }
    int fd, len, err, total = 0;
    char tmp[128] = {0}, buffer[819200];
    BIO_get_fd(client, &fd);

    struct sockaddr_in peer;
    socklen_t solen = sizeof(peer);
    memset(&peer, 0, solen);
    getpeername(fd, (struct sockaddr *) &peer, &solen);
    fd = get_tmpfile(tmp, 0666, NULL, NULL);
    printf("Connection opened from %s:%d write to %s\n",
           inet_ntoa(peer.sin_addr), ntohs(peer.sin_port), tmp);

    do {
        if ((err = BIO_read(client, buffer, sizeof(buffer))) <= 0) {
            if (err == -1) {
                SSL_print_err("BIO_read");
            }
            break ;
        }

        printf("read %d bytes:\n", err);

        BIO_write(client, "OK\r\n", 4);
        write(fd, buffer, err);
        total += err;
    } while(err > 0);
    BIO_free_all(client);

    if (fd > 0) close(fd);

    return NULL;
}

int main(int argc, char *argv[])
{
    const char *cert = "ca.cert";
    const char *prikey = cert;
    char *pass = NULL, tmp[256];
    static struct option _options[] = {
        {"pass", 1, 0, 0},
        {"cert", 1, 0, 0},
        {"key", 1, 0, 0},
        {0, 0, 0, 0}
    };

    int index = 0;
    const char* optlist = "p:";
    while (1){
        int c = getopt_long(argc, argv, optlist, _options, &index);
        if (c == EOF) break;
        switch (c) {
        case 'p':
            port = atoi(optarg);
            break;
        case 0:
            if (strcmp(_options[index].name, "pass") == 0) {
                pass = strdup (optarg);
            } else if (strcmp(_options[index].name, "key") == 0) {
                prikey = strdup (optarg);
            } else if (strcmp(_options[index].name, "cert") == 0) {
                cert = strdup (optarg);
            }
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
    SSL_CTX *ctx = SSLnew_server_ctx(cert, prikey, pass);

    SSL *ssl = NULL;
    BIO *acc, *client;

    // create a filter SSL BIO
    client = BIO_new_ssl(ctx, 0);
    BIO_get_ssl(client, &ssl);
    if (ssl == NULL)
        return 1;
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    // acc is a source/sink BIO
    sprintf(tmp, "%d", port);
    if ((acc = BIO_new_accept(tmp)) == NULL)
        return 1;
    BIO_set_bind_mode(acc, BIO_BIND_REUSEADDR);

    // when a new connection is acceptede on 'acc'
    // the 'client' will be 'dupilcated' and have the new socket BIO push into it.
    BIO_set_accept_bios(acc,client);

    // set up the accept BIO
    if (BIO_do_accept(acc) <= 0) {
        SSL_print_err("init accept");
        return 1;
    }

    printf("Server started, waiting for connection\n");
    for(;;)
    {
        // wait for incoming connection
        if (BIO_do_accept(acc) <= 0) {
            SSL_print_err("accept");
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
