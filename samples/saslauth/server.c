#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <sasl.h>

#include "common.h"

/**
 * Callbacks for Server
 */

int mygetopt(void *context, const char *plugin_name,
	     const char *option,
	     const char **result, unsigned *len)
{
    return 0;
}

int mygetpath(void *context, const char **path)
{
    return 0;
}

int mygetconfpath(void *context, char **path)
{
    char temp[1024] = {0};
    sprintf(temp, "%s:%s",
	    "/etc/sasl2",
	    getenv("PWD"));

    printf("!! Set SASL config path -> %s\n", temp);
    *path = strdup(temp);

    return 0;
}

static sasl_callback_t callbacks[] = {
//    { SASL_CB_GETOPT, &mygetopt, NULL }, 
//    { SASL_CB_GETPATH, &mygetpath, NULL }, 
    { SASL_CB_GETCONFPATH, &mygetconfpath, NULL }, 
    { SASL_CB_LIST_END, NULL, NULL }
};

/* create a socket listening on port 'port' */
/* if af is PF_UNSPEC more than one socket may be returned */
/* the returned list is dynamically allocated, so caller needs to free it */

int *listensock(const char *port, const int af)
{
    struct addrinfo hints, *ai, *r;
    int err, maxs, *sock, *socks;
    const int on = 1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = af;
    hints.ai_socktype = SOCK_STREAM;
    err = getaddrinfo(NULL, port, &hints, &ai);
    if (err) {
	fprintf(stderr, "%s\n", gai_strerror(err));
	exit(0);
    }

    /* Count max number of sockets we may open */
    for (maxs = 0, r = ai; r; r = r->ai_next, maxs++)
	;
    socks = malloc((maxs + 1) * sizeof(int));
    socks[0] = 0;
    sock = socks + 1;
    for (r = ai; r; r = r->ai_next) {
	*sock = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
	if (*sock < 0) {
	    perror("socket");
	    continue;
	}
	if (setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, (void *) &on, sizeof(on)) < 0) {
	    close(*sock);
	    continue;
	}

	if (bind(*sock, r->ai_addr, r->ai_addrlen) < 0) {
	    close(*sock);
	    continue;
 	}
 	if (listen(*sock, 5) < 0) {
 	    close(*sock);
 	    continue;
 	}

 	socks[0]++;
 	sock++;
    }

    freeaddrinfo(ai);

    if (socks[0] == 0) {
 	fprintf(stderr, "!! Couldn't bind to any socket\n");
 	free(socks);
	exit(-1);
    }

    return socks;
}

void usage(void)
{
    fprintf(stderr, "usage: server [-p port] [-s service] [-m mech]\n");
    exit(0);
}

/* globals because i'm lazy */
char *mech;

/* do the sasl negotiation; return -1 if it fails */
int mysasl_negotiate(FILE *in, FILE *out, sasl_conn_t *conn)
{
    char buf[8192];
    char chosenmech[128];
    const char *data;
    unsigned int len = 0;
    int r = SASL_FAIL;
    const char *userid;
    
    /* generate the capability list */
    if (mech) {
	data = strdup(mech);
	len = strlen(data);
    } else {
	int count = 0;

	r = sasl_listmech(conn, NULL, NULL, " ", NULL, &data, &len, &count);
	if (r != SASL_OK) saslfail(r, "Generating mechanism list", NULL);
    }

    /* send capability list to client */
    send_string(out, data, len);

    len = recv_string(in, chosenmech, sizeof chosenmech);
    if (len <= 0) {
	printf("!! Client didn't choose mechanism\n");
	fputc('N', out); /* send NO to client */
	fflush(out);
	return -1;
    }

    if (mech && strcasecmp(mech, chosenmech)) {
	printf("!! Client didn't choose mandatory mechanism\n");
	fputc('N', out); /* send NO to client */
	fflush(out);
	return -1;
    }

    len = recv_string(in, buf, sizeof(buf));
    if(len != 1) {
	saslerr(r, "Receive first-send parameter", conn);
	fputc('N', out);
	fflush(out);
	return -1;
    }

    if(buf[0] == 'Y') {
        /* receive initial response (if any) */
        len = recv_string(in, buf, sizeof(buf));

        /* start libsasl negotiation */
        r = sasl_server_start(conn, chosenmech, buf, len,
			      &data, &len);
    } else {
	r = sasl_server_start(conn, chosenmech, NULL, 0,
			      &data, &len);
    }
    
    if (r != SASL_OK && r != SASL_CONTINUE) {
	saslerr(r, "Starting SASL negotiation", conn);
	fputc('N', out); /* send NO to client */
	fflush(out);
	return -1;
    }

    while (r == SASL_CONTINUE) {
	if (data) {
	    fputc('C', out); /* send CONTINUE to client */
	    send_string(out, data, len);
	} else {
	    fputc('C', out); /* send CONTINUE to client */
	    send_string(out, "", 0);
	}

	len = recv_string(in, buf, sizeof buf);
	if (len < 0) {
	    printf("Client disconnected\n");
	    return -1;
	}

	r = sasl_server_step(conn, buf, len, &data, &len);
	if (r != SASL_OK && r != SASL_CONTINUE) {
	    saslerr(r, "Performing SASL negotiation", conn);
	    fputc('N', out); /* send NO to client */
	    fflush(out);
	    return -1;
	}
    }

    if (r != SASL_OK) {
	saslerr(r, "incorrect authentication", conn);
	fputc('N', out); /* send NO to client */
	fflush(out);
	return -1;
    }

    fputc('O', out); /* send OK to client */
    fflush(out);

    r = sasl_getprop(conn, SASL_USERNAME, (const void **) &userid);
    printf("successful authentication '%s'\n", userid);

    return 0;
}

int main(int argc, char *argv[])
{
    int c;
    char *port = "12345";
    char *service = "rcmd";
    int *l, maxfd=0;
    int r, i;
    sasl_conn_t *conn;

    while ((c = getopt(argc, argv, "p:s:m:")) != EOF) {
	switch(c) {
	case 'p':
	    port = optarg;
	    break;

	case 's':
	    service = optarg;
	    break;

	case 'm':
	    mech = optarg;
	    break;

	default:
	    usage();
	    break;
	}
    }

    /* initialize the sasl library */
    r = sasl_server_init(callbacks, "sample");
    if (r != SASL_OK) saslfail(r, "Initializing libsasl", NULL);

    /* get a listening socket */
    if ((l = listensock(port, PF_UNSPEC)) == NULL) {
	fprintf(stderr, "!! No socket opened\n");
	return -1;
    }

    for (i = 1; i <= l[0]; i++) {
       if (l[i] > maxfd)
           maxfd = l[i];
    }

    for (;;) {
	char localaddr[NI_MAXHOST | NI_MAXSERV],
	     remoteaddr[NI_MAXHOST | NI_MAXSERV];
	char myhostname[1024+1];
	char hbuf[NI_MAXHOST], pbuf[NI_MAXSERV];
	struct sockaddr_storage local_ip, remote_ip;
	int niflags;
	socklen_t salen;
	int nfds, fd = -1;
	FILE *in, *out;
	fd_set readfds;

	FD_ZERO(&readfds);
	for (i = 1; i <= l[0]; i++)
	    FD_SET(l[i], &readfds);

	nfds = select(maxfd + 1, &readfds, 0, 0, 0);
	if (nfds <= 0) {
	    if (nfds < 0 && errno != EINTR)
		perror("select");
	    continue;
	}

       for (i = 1; i <= l[0]; i++) 
           if (FD_ISSET(l[i], &readfds)) {
               fd = accept(l[i], NULL, NULL);
               break;
           }

	if (fd < 0) {
	    if (errno != EINTR)
		perror("accept");
	    continue;
	}

	/* set ip addresses */
	niflags = (NI_NUMERICHOST | NI_NUMERICSERV);
	salen = sizeof(local_ip);
	getsockname(fd, (struct sockaddr *)&local_ip, &salen);
	getnameinfo((struct sockaddr *)&local_ip, salen, hbuf, sizeof(hbuf), pbuf, sizeof(pbuf), niflags);
        snprintf(localaddr, sizeof(localaddr), "%s;%s", hbuf, pbuf);

	salen = sizeof(remote_ip);
	getpeername(fd, (struct sockaddr *)&remote_ip, &salen);
	getnameinfo((struct sockaddr *)&remote_ip, salen, hbuf, sizeof(hbuf), pbuf, sizeof(pbuf), niflags);
	snprintf(remoteaddr, sizeof(remoteaddr), "%s;%s", hbuf, pbuf);

	r = gethostname(myhostname, sizeof(myhostname)-1);

	printf("!! Accepted new connection, hostname:%s local:%s remote:%s\n", myhostname, localaddr, remoteaddr);

	r = sasl_server_new(service, myhostname, NULL, localaddr, remoteaddr,
			    NULL, 0, &conn);
	in = fdopen(fd, "r");
	out = fdopen(fd, "w");

	r = mysasl_negotiate(in, out, conn);
	if (r == SASL_OK) {
	    char buf[1024] = {0};
	    recv_string(in, buf, sizeof(buf));
	}

	printf("!! Closing connection\n");
	fclose(in);
	fclose(out);
	close(fd);
	sasl_dispose(&conn);
    }

    sasl_done();
}
