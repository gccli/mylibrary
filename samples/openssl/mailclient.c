#include <sys/types.h>
#include <sys/syscall.h>
#include <assert.h>
#include <getopt.h>

#include "sslcommon.h"

char *host;
int   port;

int main(int argc, char *argv[])
{
	static struct option long_options[] = {
		{0, 0, 0, 0}
	};
	int index = 0;
	const char* optlist = "h:p:";
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
		case 0:
			break;
		default:
			printf("usage: %s [-h host] [-p port]\n", argv[0]);
			exit(0);
		}
	}

	SSLinit();

	char peer[128] = {0};
	sprintf(peer, "%s:%d", host, port);
	BIO *conn = BIO_new_connect(peer);
	if (BIO_do_connect(conn) <= 0) {
		SSL_LOGERR("connect");
		return 1;
	}
	int  total = 0;
	int  err, nwritten;
	char buf[80];
	for (;;)
	{
		if (!fgets(buf, sizeof(buf), stdin))
			break;

		int length = strlen(buf);
		for (nwritten = 0; nwritten < length; nwritten += err)
		{
			err = BIO_write(conn, buf + nwritten, length-nwritten);
			if (err <= 0) {
				SSL_LOGERR("BIO_write");
				return 1;
			}
		}
		total += nwritten;
	}

	fprintf(stderr, "Connection opened, %d bytes written.\n", total);


	BIO_free(conn);

	return 0;
}

