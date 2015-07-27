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
#include <assert.h>

void *ipv6_thread(void *param)
{
	int sock = *(int *) param;
	struct sockaddr_storage ss;
	memset(&ss, 0, sizeof(ss));
	socklen_t addrlen = sizeof(ss);
	if (getpeername(sock, (struct sockaddr *) &ss, &addrlen) != 0)
		perror("getpeername()");

	char host[96] = {0};
	char serv[32] = {0};
	int flag = 0;	
	flag = NI_NUMERICHOST | NI_NUMERICSERV; // tcpdump -nn
	if (getnameinfo((struct sockaddr *) &ss, addrlen, host, sizeof(host), serv, sizeof(serv), flag) != 0)
		perror("getpeername()");

	struct sockaddr_in6 *a6 = (struct sockaddr_in6 *) &ss;
	printf("Receive %s Client %s/%s\n", IN6_IS_ADDR_V4MAPPED(&a6->sin6_addr)?"IPv4":"IPv6", host, serv);

	send(sock, "HELLO\r\n", 7, 0);

	char buffer[1024];
	struct timeval tv;
	fd_set rset;

	while(1)
	{
		FD_ZERO(&rset);
		FD_SET(sock, &rset);
		tv.tv_sec = 2; tv.tv_usec = 0;
		int count = select (sock+1, &rset, NULL, NULL, &tv);
		if (count < 0) break ;
		else if (count == 0)
			continue;
		int len = recv(sock, buffer, sizeof(buffer), 0);
		if (len <= 0)
			break;
		buffer[len] = 0;
		if (strncmp(buffer, "quit", 4) == 0) 
			break;
		printf("%s", buffer);
		send(sock, "* OK ", 5, 0);
		send(sock, buffer, len, MSG_NOSIGNAL);
	}
	printf("client quit\n");

	close(sock);
	return NULL;
}


int main(int argc, char *argv[])
{
	struct addrinfo hints, *serv6 = NULL;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM; // just for tcp server
	int err = getaddrinfo(argv[1], "3200", &hints, &serv6);
	if (err != 0)
	{
		fprintf (stderr, "getaddrinfo error: %d (%s)\n", err, gai_strerror(err));
		return 1;
	}

	printf("sizeof(struct sockaddr_in6) = %d, sizeof(struct sockadd) = %d addrlen = %d\n",
		sizeof(struct sockaddr_in6), sizeof(struct sockaddr), serv6->ai_addrlen);

	//===================================================================================
	int sock = socket(serv6->ai_family, serv6->ai_socktype, 0);
	int enable = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) 
	{
		fprintf (stderr, "setsockopt(SO_REUSEADDR) error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}

	if (bind(sock, serv6->ai_addr, serv6->ai_addrlen) < 0)
	{
		fprintf(stderr, "failed to bind ipv6 address: %s\n", strerror(errno));
		close(sock);
		return 1;
	}
	listen(sock, 5);

	while(1) {
	    struct sockaddr_in cliaddr;
	    socklen_t alen = sizeof(cliaddr);

	    int clisock = accept(sock, (struct sockaddr *) &cliaddr, &alen);
		if (clisock < 0) {
			sleep(1);
			fprintf(stderr, "failed to accept: %s\n", strerror(errno));
			continue;
		}
		printf("CLIENT:%s\n", inet_ntoa(cliaddr.sin_addr));


		pthread_t th;
		pthread_attr_t thattr;
		pthread_attr_init(&thattr);
		pthread_attr_setdetachstate(&thattr,PTHREAD_CREATE_DETACHED);
		pthread_create(&th, &thattr, ipv6_thread, &clisock);		
	}

	return 0;
}


