#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <netdb.h>
#include <pthread.h>
#include <sys/time.h>
static int verbose = 1;
struct addrinfo *gethostaddr(const char *hostname, const char *service)
{
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_ALL | AI_CANONNAME;
	hints.ai_socktype = SOCK_STREAM; // just for tcp server

	struct addrinfo *addr = NULL;
	int err = getaddrinfo(hostname, service, &hints, &addr);
	if (err != 0)
	{
		fprintf (stderr, "getaddrinfo error: %d (%s)\n", err, gai_strerror(err));
		return NULL;
	}

	struct addrinfo *p = addr;
	for (; verbose && p ; p = p->ai_next) {
		void *pdata;
		if (p->ai_family == AF_INET) {
			pdata = (void *) &((struct sockaddr_in *) p->ai_addr)->sin_addr;
		}
		else if (p->ai_family == AF_INET6) {
			pdata = (void *) &((struct sockaddr_in6 *) p->ai_addr)->sin6_addr;
		}

		char ip[INET6_ADDRSTRLEN] = {0};
		printf("Family:%-2d Socket Type:%-2d Protocol Type:%-2d Canonical Name:%-16s Address:%s\n",
			p->ai_family, p->ai_socktype, p->ai_protocol, p->ai_canonname, inet_ntop(p->ai_family, pdata, ip, sizeof(ip)));
	}

	return addr;
}

int main(int argc, char *argv[])
{
	struct addrinfo *p, *dstaddr;
	dstaddr = gethostaddr(argv[1], argv[2]);
	if (dstaddr == NULL)
		return 1;
	p = dstaddr;

	int sock, ret = -1;
	for (;p ; p = p->ai_next)
	{
		sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		ret = connect(sock, p->ai_addr, p->ai_addrlen);
		if (ret == 0) {
			char ip[INET6_ADDRSTRLEN] = {0};
			void *pdata;
			if (p->ai_family == AF_INET) {
				pdata = (void *) &((struct sockaddr_in *) p->ai_addr)->sin_addr;
			}
			else if (p->ai_family == AF_INET6) {
				pdata = (void *) &((struct sockaddr_in6 *) p->ai_addr)->sin6_addr;
			}			
			printf("Connect to %s successfully\n", inet_ntop(p->ai_family, pdata, ip, sizeof(ip)));
			break;
		}
		close(sock);
	}
	if (ret < 0)
	{
		fprintf(stderr, "failed to connect server: %s", strerror(errno));
		return 1;
	}
	freeaddrinfo (dstaddr);


	char line[128] = {0};
	strcpy(line, "GET / HTTP/1.1\r\n\r\n");
	send(sock, line, strlen(line), 0);

	struct timeval tv;
	fd_set rset;

	while (1) {
		FD_ZERO(&rset);
		FD_SET(sock, &rset);

		tv.tv_sec = 2; tv.tv_usec = 0;
		if (select (sock+1, &rset, NULL, NULL, &tv) > 0) 
		{
			char buffer[1024];
			int len = recv(sock, buffer, sizeof(buffer), 0);
			if (len <= 0)
				break;
			buffer[len] = 0;
			printf("%s", buffer);
		}
		else break;
	}
	close(sock);

	getchar();

	return 0;
}

