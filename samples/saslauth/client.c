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

#include <assert.h>

#include <sasl.h>

#include "common.h"

/* remove \r\n at end of the line */
static void chop(char *s)
{
    char *p;

    assert(s);
    p = s + strlen(s) - 1;
    if (p[0] == '\n') {
	*p-- = '\0';
    }
    if (p >= s && p[0] == '\r') {
	*p-- = '\0';
    }
}

static int getrealm(void *context __attribute__((unused)), 
		    int id,
		    const char **availrealms,
		    const char **result)
{
    static char buf[1024];

    /* paranoia check */
    if (id != SASL_CB_GETREALM) return SASL_BADPARAM;
    if (!result) return SASL_BADPARAM;

    printf("please choose a realm (available:");
    while (*availrealms) {
	printf(" %s", *availrealms);
	availrealms++;
    }
    printf("): ");

    fgets(buf, sizeof(buf), stdin);
    chop(buf);
    *result = buf;
  
    return SASL_OK;
}

static int simple(void *context __attribute__((unused)),
		  int id,
		  const char **result,
		  unsigned *len)
{
    static char buf[1024];

    /* paranoia check */
    if (!result)
	return SASL_BADPARAM;

    switch (id) {
	case SASL_CB_USER:
	    printf("User> ");
	    break;
	case SASL_CB_AUTHNAME:
	    printf("Authname> ");
	    break;
	default:
	    return SASL_BADPARAM;
    }

    fgets(buf, sizeof(buf), stdin);
    chop(buf);
    *result = buf;
    if (len) *len = strlen(buf);
    return SASL_OK;
}

static int
getsecret(sasl_conn_t *conn,
	  void *context __attribute__((unused)),
	  int id,
	  sasl_secret_t **psecret)
{
    char *password;
    size_t len;
    static sasl_secret_t *x;

    /* paranoia check */
    if (! conn || ! psecret || id != SASL_CB_PASS)
	return SASL_BADPARAM;

    password = getpass("Password> ");
    if (!password)
	return SASL_FAIL;

    len = strlen(password);

    x = (sasl_secret_t *) realloc(x, sizeof(sasl_secret_t) + len);
  
    if (!x) {
	memset(password, 0, len);
	return SASL_NOMEM;
    }

    x->len = len;
    memcpy(x->data, password, len);
    memset(password, 0, len);
    
    *psecret = x;
    return SASL_OK;
}


/* callbacks we support */
static sasl_callback_t callbacks[] = {
  {
    SASL_CB_GETREALM, &getrealm, NULL
  }, {
    SASL_CB_USER, &simple, NULL
  }, {
    SASL_CB_AUTHNAME, &simple, NULL
  }, {
    SASL_CB_PASS, &getsecret, NULL
  }, {
    SASL_CB_LIST_END, NULL, NULL
  }
};

int getconn(const char *host, const char *port)
{
    struct addrinfo hints, *ai, *r;
    int err, sock = -1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((err = getaddrinfo(host, port, &hints, &ai)) != 0) {
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
	exit(1);
    }

    for (r = ai; r; r = r->ai_next) {
	sock = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
	if (sock < 0)
	    continue;
	if (connect(sock, r->ai_addr, r->ai_addrlen) >= 0)
	    break;
	close(sock);
	sock = -1;
    }

    freeaddrinfo(ai);
    if (sock < 0) {
	perror("connect");
	exit(1);
    }

    return sock;
}

char *mech;

int mysasl_negotiate(FILE *in, FILE *out, sasl_conn_t *conn)
{
    char buf[8192];
    const char *data;
    const char *chosenmech = NULL;
    unsigned int len = 0;
    int r, c;

    /* get the capability list */
    len = recv_string(in, buf, sizeof(buf));

    if (mech) {
	/* make sure that 'mech' appears in 'buf' */
	if (!strstr(buf, mech)) {
	    printf("!! Server doesn't offer mandatory mech '%s'\n", mech);
	    return -1;
	}
    } else {
	mech = buf;
    }

    r = sasl_client_start(conn, mech, NULL, &data, &len, &chosenmech);
    if (r != SASL_OK && r != SASL_CONTINUE) {
	saslerr(r, "Starting SASL negotiation", conn);
	return -1;
    }
    /* we send up to 3 strings;
       the mechanism chosen, the presence of initial response,
       and optionally the initial response */
    send_string(out, chosenmech, strlen(chosenmech));
    if(data) {
	send_string(out, "Y", 1);
	send_string(out, data, len);
    } else {
	send_string(out, "N", 1);
    }

    for (;;) {

	c = fgetc(in);
	switch (c) {
	case 'O':
	    goto done_ok;

	case 'N':
	    goto done_no;

	case 'C': /* continue authentication */
	    break;

	default:
	    printf("!! Bad protocol from server (%c %x)\n", c, c);
	    return -1;
	}
	len = recv_string(in, buf, sizeof(buf));

	r = sasl_client_step(conn, buf, len, NULL, &data, &len);
	if (r != SASL_OK && r != SASL_CONTINUE) {
	    saslerr(r, "Performing SASL negotiation", conn);
	    return -1;
	}

	if (data) {
	    send_string(out, data, len);
	} else {
	    send_string(out, "", 0);
	}
    }

 done_ok:
    printf("!! Successful authentication\n");
    return 0;

 done_no:
    printf("!! Authentication failed\n");
    return -1;
}

void usage(void)
{
    fprintf(stderr, "usage: client [-p port] [-s service] [-m mech] host\n");
    exit(0);
}

int main(int argc, char *argv[])
{
    int c;
    char *host = "localhost";
    char *port = "12345";
    char *service = "rcmd";
    char localaddr[NI_MAXHOST + NI_MAXSERV], remoteaddr[NI_MAXHOST + NI_MAXSERV];
    char hbuf[NI_MAXHOST], pbuf[NI_MAXSERV];

    int r;
    sasl_conn_t *conn;
    FILE *in, *out;
    int fd;
    socklen_t salen;
    int niflags;
    struct sockaddr_storage local_ip, remote_ip;

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

    if (optind > argc - 1) {
	usage();
    }
    if (optind == argc - 1) {
	host = argv[optind];
    }

    /* initialize the sasl library */
    r = sasl_client_init(callbacks);
    if (r != SASL_OK) saslfail(r, "Initializing libsasl", NULL);

    /* connect to remote server */
    fd = getconn(host, port);

    /* set ip addresses */
    salen = sizeof(local_ip);
    getsockname(fd, (struct sockaddr *)&local_ip, &salen);

    niflags = (NI_NUMERICHOST | NI_NUMERICSERV);
    getnameinfo((struct sockaddr *)&local_ip, salen, hbuf, sizeof(hbuf), pbuf, sizeof(pbuf), niflags);
    snprintf(localaddr, sizeof(localaddr), "%s;%s", hbuf, pbuf);

    salen = sizeof(remote_ip);
    getpeername(fd, (struct sockaddr *)&remote_ip, &salen);

    niflags = (NI_NUMERICHOST | NI_NUMERICSERV);
    getnameinfo((struct sockaddr *)&remote_ip, salen, hbuf, sizeof(hbuf), pbuf, sizeof(pbuf), niflags);
    snprintf(remoteaddr, sizeof(remoteaddr), "%s;%s", hbuf, pbuf);
    
    printf("!! Local address %s, Remote address %s\n", localaddr, remoteaddr);

    /* client new connection */
    r = sasl_client_new(service, host, localaddr, remoteaddr, NULL, 0, &conn);
    in = fdopen(fd, "r");
    out = fdopen(fd, "w");

    r = mysasl_negotiate(in, out, conn);
    if (r == SASL_OK) {
	send_string(out, "HELLO", 5);	
    }
    
    printf("!! Closing connection\n");
    fclose(in);
    fclose(out);
    close(fd);
    sasl_dispose(&conn);
    sasl_done();

    return 0;
}
