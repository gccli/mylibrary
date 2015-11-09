#include <sys/types.h>
#include <sys/syscall.h>
#include <assert.h>
#include <getopt.h>

#include "sslcommon.h"

char *host = "localhost";
int   port = 3200;
int   flags;


int verify_callback(int ok, X509_STORE_CTX *store)
{
	char data[256];
	if (!ok)
	{
		X509 *cert = X509_STORE_CTX_get_current_cert(store);
		int depth = X509_STORE_CTX_get_error_depth(store);
		int err = X509_STORE_CTX_get_error(store);
		fprintf(stderr, "-Error with certificate at depth: %i\n", depth);
		X509_NAME_oneline(X509_get_issuer_name(cert), data, 256);
		fprintf(stderr, " issuer = %s\n", data);
		X509_NAME_oneline(X509_get_subject_name(cert), data, 256);
		fprintf(stderr, " subject = %s\n", data);
		fprintf(stderr, " error %i:%s\n", err, X509_verify_cert_error_string(err));
	}
	return ok;
}

int main(int argc, char *argv[])
{
	static struct option long_options[] = {
		{0, 0, 0, 0}
	};
	int index = 0;
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
	SSL_CTX *ctx = SSL_CTX_new(TLSv1_method());

	// set default locations for trusted CA certificates
	if (SSL_CTX_load_verify_locations(ctx, NULL, "certs") <= 0) {
		SSL_LOGERR("load verify");
		return 1;
	}

	// SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_callback);

	conn = BIO_new_ssl_connect(ctx);
	BIO_get_ssl(conn, &ssl);
	if (ssl == NULL)
		return -1;

	char strport[8] = {0};
	sprintf(strport, "%d", port);

	BIO_set_conn_hostname(conn, host);
	BIO_set_conn_port(conn, strport);
	// BIO_set_nbio(conn, 1);
	SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

	if (BIO_do_connect(conn) <= 0) {
		SSL_LOGERR("connect");
		return 1;
	}

	if (BIO_do_handshake(conn) <=0 ) {
		SSL_LOGERR("handshake");
		return 1;
	}

	SSL_SESSION *session = SSL_get_session(ssl);
	if (session == NULL) {
		printf("no session\n");
	}

	int sock = 0;
	BIO_get_fd(conn, &sock);

	int  total = 0;
	int  wlen, rlen;
	int  nbuflen, nread, nwritten;
	char sndbuf[1024] = {0};
	char rcvbuf[4096*1024] = {0};

	rlen = BIO_read(conn, rcvbuf, sizeof(rcvbuf));
	if (rlen <= 0) {
		return 1;
	}
	rcvbuf[rlen] = 0;
	fprintf(stderr, "SSL connection opened, server response: %s", rcvbuf);

	for (;;)
	{
		if (!fgets(sndbuf, sizeof(sndbuf), stdin))
			break;
		int i=strlen(sndbuf)-1;
		for (; i>=0; --i) {
			if (sndbuf[i] == 0x0a) sndbuf[i] = 0;
			else break;
		}
		strcat(sndbuf, "\r\n");

		int length = strlen(sndbuf);
		for (nwritten = 0; nwritten < length; nwritten += wlen)
		{
			wlen = BIO_write(conn, sndbuf + nwritten, length-nwritten);
			if (wlen <= 0) {
				SSL_LOGERR("BIO_write");
				return 1;
			}
		}
		total += nwritten;

		// get IMAP tag
		for (i=0; i<strlen(sndbuf); i++)
			if (sndbuf[i] == 0x20) {
				sndbuf[i] = 0;
				break;
			}

		// read response
		for(nread=0, nbuflen=sizeof(rcvbuf); flags ;nread += rlen ) {
			rlen = BIO_read(conn, rcvbuf+nread, nbuflen-nread);
			if (rlen < 0) {
				SSL_LOGERR("BIO_read");
				break;
			}
			if (strstr(rcvbuf+nread, sndbuf)) { // find the tag
				nread += rlen;
				break;
			}
		}
		if (flags) {
			rcvbuf[nread] = 0;
			fprintf(stderr, "server response: %s", rcvbuf);
		}
	}

	BIO_free(conn);

	return 0;
}
