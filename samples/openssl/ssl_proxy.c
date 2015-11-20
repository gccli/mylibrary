
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

// Server side write buffer
    char *csbuf, *csbuf_b, *csbuf_e;

// Client side write buffer
    char *scbuf, *scbuf_b, *scbuf_e;
} Conn;
Conn *conn=NULL;

void conn_close_client(Conn *conn);
void conn_close_server(Conn *conn);

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

void plog_ssl_error(SSL *ssl_conn, int ret, char *cls, int sock)
{
    int err=SSL_get_error(ssl_conn, ret);
    switch (err) {
	case SSL_ERROR_NONE:
	case SSL_ERROR_WANT_READ:
	case SSL_ERROR_WANT_WRITE:
	case SSL_ERROR_WANT_CONNECT:
	case SSL_ERROR_WANT_ACCEPT:
	    break;
	case SSL_ERROR_SSL:
	    plog(LOG_ERR, "ERROR @%d %s: %s", sock, cls,
		    ERR_error_string(ERR_get_error(), NULL));
	    break;
	case SSL_ERROR_SYSCALL:
	    if (!ret) {
		plog(LOG_ERR, "ERROR @%d %s: Unexpected EOF", sock, cls);
	    } else {
		plog(LOG_ERR, "ERROR @%d %s: %s (errno=%d)", sock, cls, strerror(errno), errno);
	    }
	    break;
	case SSL_ERROR_ZERO_RETURN:
//	    plog(LOG_ERR, "ERROR @%d %s: Zero return", sock, cls);
	    break;
	default:
	    plog(LOG_ERR, "ERROR @%d %s: Unknown SSL error (SSL_get_error()=%d)", sock, cls, err);
	    break;
    }
}

void _sleep()
{
    struct timeval tv={0, SLEEP_US};
    select(0, NULL, NULL, NULL, &tv);
}


int serverside_init(int port)
{
    char tmp[128];

    server_ctx = SSLnew_server_ctx("certs/server.pem", "lijing");
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
    int err, total;
    char buffer[819200];
    char csbuf[8192];

    if (BIO_do_handshake(conn->ss_client) <= 0) {
        SSL_LOGERR("thread handshake");
        goto error;
    }

    BIO_set_nbio(conn->ss_client, 1);
    BIO_set_nbio(conn->cs_bio, 1);

    do
    {
        // Read server greeting
        err = BIO_nb_read(conn->cs_bio, csbuf, sizeof(csbuf));
        if (err > 0){
            err = BIO_nb_write(conn->ss_client, csbuf, err);
        }

        err = encrypt_s(enc_ctx, conn->ss_client, conn->cs_bio);
        if (err != 0) {
            printf("encrypt stream fail\n");
        }

        /*
        // Read from client
        if ((err = BIO_nb_read(conn->ss_client, buffer, sizeof(buffer))) < 0) {
            printf("client close\n");
            break ;
        }

        if (err > 0) {
            err = BIO_nb_write(conn->cs_bio, buffer, err);
            if (err < 0) {
                printf("failed to read\n");
            } else {
                printf("proxy write %d\n", err);
            }
            }*/



        total += err;
    } while(err >= 0);

    fprintf(stderr, "Connection closed, %d bytes received.\n", total);

error:
    BIO_free_all(conn->ss_client);
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
    client_ctx = SSLnew_client_ctx("certs");
    serverside_init(4000);

    while(1) {
        ret = conn_accept();
        if (ret != 0) break;
    }
    return 0;
}
/*

    while (1) {
	int event=0, ci;
	// Check for incoming connections
	if ((i=conn_accept())>0) {
	    debug("Client connected");
	    event=1;
	}
	for (ci=0; ci<max_conn; ci++) {
	    Conn *cn=&conn[ci];
	    int l;
	    switch (cn->stat) {
            case cs_accept:
                i=conn_ssl_accept(cn);
                event|=(i==0);
                break;
            case cs_connecting: {
                struct sockaddr_in client_addr;
                unsigned int client_addr_len=sizeof(client_addr);
                X509 *cert;
                X509_NAME *xn=NULL;
                char peer_cn[256]="";
                getpeername(cn->server_sock,
                            (struct sockaddr *)&client_addr,
                            &client_addr_len);
                cert=SSL_get_peer_certificate(cn->ssl_conn);
                if (cert) {
                    xn=X509_get_subject_name(cert);
                    X509_NAME_get_text_by_NID(xn, NID_commonName, peer_cn, 256);
                }

                cn->csbuf_e+=snprintf(cn->csbuf_b, cs_buflen,
                                      "#@ip=%s port=%d%s%s%s\r\n",
                                      inet_ntoa(client_addr.sin_addr),
                                      htons(client_addr.sin_port), xn?" cn='":"", peer_cn, xn?"'":"");
                debug("INFO: %p %d %s", cn, cn->server_sock, cn->csbuf);

                plog(LOG_INFO, "CONNECT @%d %s:%d%s%s%s",
                     cn->server_sock, inet_ntoa(client_addr.sin_addr), htons(client_addr.sin_port),
                     xn?" cn='":"", peer_cn, xn?"'":"");
                cn->stat=cs_connected;
            }
                break;
            case cs_connected:
                if ((l=cs_buflen-(cn->csbuf_e-cn->csbuf))) {
                    i=SSL_read(cn->ssl_conn, cn->csbuf_e, l);
                    if (i<=0) { // Error, or shutdown
                        if (errno!=EAGAIN) {
                            plog_ssl_error(cn->ssl_conn, i, "SSL_read()",
                                           conn->server_sock);
                            cn->stat=cs_closing; event=1;
                        }
                    } else cn->csbuf_e+=i;
                }
            case cs_closing:
                // Send buffered data to server
                if ((l=cn->csbuf_e-cn->csbuf_b)>0) {
                    i=write(cn->client_sock, cn->csbuf_b, l); event=1;
                    //write(2, cn->csbuf_b, l);
                    if (i>=0) {
                        cn->csbuf_b+=i;
                    } else {
			    if (errno!=EAGAIN) {
				plog(LOG_ERR, "ERROR @%d write(): %s",
					conn->server_sock, strerror(errno));
				cn->csbuf_b=cn->csbuf_e=cn->csbuf;
				cn->stat=cs_closing;
			    }
			}
			if (cn->csbuf_b==cn->csbuf_e) {
			    cn->csbuf_b=cn->csbuf_e=cn->csbuf;
//			    if (cn->c_end_req) conn_close(cn);
			}
		    }
		    if (cn->stat==cs_closing && cn->csbuf_e==cn->csbuf_b) conn_close_client(cn);
		default:;
	    }
	    if (cn->stat==cs_connected || cn->stat==cs_closing) {
		// Check if data is available on server side
		if ((l=sc_buflen-(cn->scbuf_e-cn->scbuf)) && cn->client_sock>=0) {
		    i=read(cn->client_sock, cn->scbuf_e, l);
		    if (!i) { // End of connection
			cn->stat=cs_closing; event=1;
//			cn->s_end_req=1;
		    } else if (i<0) { // Error
			if (errno!=EAGAIN) {
			    plog(LOG_ERR, "ERROR @%d read(): %s",
				    conn->server_sock, strerror(errno));
			    cn->stat=cs_closing; event=1;
//			    cn->s_end_req=1;
			}
		    } else cn->scbuf_e+=i;
		}
		// Send buffered data to client
		if ((l=cn->scbuf_e-cn->scbuf_b)>0 && cn->server_sock>=0) {
		    i=SSL_write(cn->ssl_conn, cn->scbuf_b, l);
		    if (i>0) debug("transfer: buf=%d, b=%d, l=%d, i=%d", cn->scbuf,
			    cn->scbuf_b, l, i);
		    if (i>=0) {
			cn->scbuf_b+=i; event=1;
		    } else if (errno!=EAGAIN) {
			plog_ssl_error(cn->ssl_conn, i, "SSL_write()",
				conn->server_sock);
			cn->scbuf_b=cn->scbuf_e=cn->scbuf;
			event=1;
		    }
		    if (cn->scbuf_b==cn->scbuf_e) {
			cn->scbuf_b=cn->scbuf_e=cn->scbuf;
//			if (cn->s_end_req) conn_close(cn);
		    }
		}
		if (cn->stat==cs_closing && cn->scbuf_e==cn->scbuf_b) conn_close_server(cn);
	    }
	}
	if (!event) _sleep();
    }
    return 0;
}
*/
