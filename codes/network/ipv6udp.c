#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <net/if.h>

#include <netdb.h>
#include <pthread.h>
#include <sys/time.h>

#include <getopt.h>

#include "utilsock.h"

/*
http://tools.ietf.org/html/rfc3493
Compatibility with IPv4 Nodes

 Applications may use AF_INET6 sockets to open TCP connections to IPv4
 nodes, or send UDP packets to IPv4 nodes, by simply encoding the
 destination's IPv4 address as an IPv4-mapped IPv6 address, and
 passing that address, within a sockaddr_in6 structure, in the
 connect() or sendto() call.
 */

static int    i6af = AF_INET6; // Use IPv6 protocol by default, only if -4 option specified
static int    i6verbose;
static int    i6mode;
static int    i6multicast;
static char  *i6grpaddr;
static int    i6mcastloop = 0;
static size_t i6length;
static char  *i6host;
static char  *i6port = "3200";
static int    i6ifindex;
static char   i6ifname[IF_NAMESIZE];




static struct addrinfo *i6gethost(const char *hostname, const char *service)
{
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = i6af;
	hints.ai_flags = AI_ALL | AI_CANONNAME;
	hints.ai_socktype = SOCK_DGRAM;
	struct addrinfo *addr = NULL;
	int err = getaddrinfo(hostname, service, &hints, &addr);
	if (err != 0)
	{
		fprintf (stderr, "getaddrinfo error: %d (%s)\n", err, gai_strerror(err));
		return NULL;
	}

	struct addrinfo *p = addr;
	for (; i6verbose && p ; p = p->ai_next) {
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

static int i6client(const char *host, const char *service)
{
	struct addrinfo *p, *dstaddr;
	dstaddr = i6gethost(host, service);
	if (dstaddr == NULL)
		return 1;

	int sock = socket(i6af, SOCK_DGRAM, IPPROTO_UDP);
	for (p = dstaddr; p ; p = p->ai_next)
		if (p->ai_family == i6af)
			break;
	if (p == NULL) 
		return 1;

	// for IPv6 option
	if (i6af == AF_INET6)
	{
		int hoplimit = 0;
		//socklen_t sklen = sizeof(hoplimit);	
		//getsockopt(sock, IPPROTO_IPV6, IPV6_UNICAST_HOPS, (char *) &hoplimit, &sklen);
		//printf("Using %d for hop limit.\n", hoplimit);
		hoplimit = 10;
		if (setsockopt(sock, IPPROTO_IPV6, IPV6_UNICAST_HOPS, (char *) &hoplimit, sizeof(hoplimit)) < 0)
			perror("setsockopt IPV6_UNICAST_HOPS");
		
	}

	char buffer[64*1024] = {0};
	size_t i;
	for (i=0; i<sizeof(buffer)-1024; i+=2) {
		sprintf(buffer+i, "%02x", (i/2)%256); 
	}
	
	i6length = i6length == 0 ? 16*1024 : (i6length > sizeof(buffer) ? sizeof(buffer): i6length);

	if (i6multicast && i6ifname[0])
	{
		if (mcast_set_if(sock, i6ifname, 0) != 0)
		{
			fprintf(stderr, "failed to set output interface: %s\n", strerror(errno));
			close(sock);
			return 1;
		}
		mcast_set_loop(sock, i6mcastloop);
	}

	int ret = sendto(sock, buffer, i6length, 0, p->ai_addr, p->ai_addrlen);
	if (ret <= 0) {
		perror("sendto");
		return -1;
	}

	struct timeval tv;
	fd_set rset;

	while (1) {
		FD_ZERO(&rset);
		FD_SET(sock, &rset);

		tv.tv_sec = 2; tv.tv_usec = 0;
		if (select (sock+1, &rset, NULL, NULL, &tv) > 0) 
		{
			int len = recvfrom(sock, buffer, sizeof(buffer), 0, NULL, NULL);
			if (len <= 0)
				break;
			buffer[len] = 0;
			printf("%s", buffer);
		}
		else break;
	}

	close(sock);
	freeaddrinfo(dstaddr);

	return ret > 0 ? 0 : -1;
}


static int i6server()
{
	struct addrinfo hints, *serv6 = NULL;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET6;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_DGRAM;
	int err = getaddrinfo(i6host, i6port, &hints, &serv6);
	if (err != 0)
	{
		fprintf (stderr, "getaddrinfo error: %d (%s)\n", err, gai_strerror(err));
		return 1;
	}

	//===================================================================================
	int sock = socket(serv6->ai_family, serv6->ai_socktype, 0);
	int enable = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) 
	{
		fprintf (stderr, "setsockopt(SO_REUSEADDR) error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}

	int  on6 = 1;
	char on = 1;
	if (setsockopt(sock, IPPROTO_IPV6, IPV6_RECVPKTINFO, &on6, sizeof(int)) < 0) 
	{
		fprintf(stderr, "setsockopt(IPV6_RECVPKTINFO): %s\n", strerror(errno));
	}
	if (setsockopt(sock, IPPROTO_IP, IP_PKTINFO, &on, sizeof(char)) < 0)
	{
		fprintf(stderr, "setsockopt(IP_PKTINFO): %s\n", strerror(errno));
	}

	if (bind(sock, serv6->ai_addr, serv6->ai_addrlen) < 0)
	{
		fprintf(stderr, "failed to bind ipv6 address: %s\n", strerror(errno));
		close(sock);
		return 1;
	}
	freeaddrinfo(serv6);

	// setup multicast 
	if (i6multicast &&  i6grpaddr && i6ifname[0])
	{
		printf("setup multicast\n");
		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_DGRAM;
		getaddrinfo(i6grpaddr, i6port, &hints, &serv6);
		if (mcast_join(sock, serv6->ai_addr, serv6->ai_addrlen, i6ifname, 0) != 0)
		{
			fprintf(stderr, "failed to join multicast group: %s\n", strerror(errno));
			close(sock);
			return 1;
		}
	}

	struct sockaddr_storage ss;
	char buffer[1024*64];
	while(1)
	{
		memset(&ss, 0, sizeof(ss));
		socklen_t addrlen = sizeof(ss);
		int flags = 0;
		struct sockaddr_storage dst;
		int len = recvfromext(sock, buffer, sizeof(buffer), &flags, (struct sockaddr *) &ss, &addrlen, &dst);
		//int len = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *) &ss, &addrlen);
		if (len < 0) {
			perror("recvfrom");
			break;
		}
		buffer[len] = 0;

		char host[96] = {0};
		char serv[32] = {0};
		int flag = 0;	
		flag = NI_NUMERICHOST | NI_NUMERICSERV; // tcpdump -nn
		if (getnameinfo((struct sockaddr *) &ss, addrlen, host, sizeof(host), serv, sizeof(serv), flag) != 0)
			perror("getpeername()");

		char ipaddr[INET6_ADDRSTRLEN];
		struct sockaddr_in6 *a6 = (struct sockaddr_in6 *) &ss;
		
		if (dst.ss_family == AF_INET6)
		{
			struct sockaddr_in6 *dst6 = (struct sockaddr_in6 *) &dst;
			printf("Receive %d bytes %s client %s/%s DESTINATION ADDR:%s\n",
				len, IN6_IS_ADDR_V4MAPPED(&a6->sin6_addr)?"IPv4":"IPv6", host, serv, inet_ntop(AF_INET6, &dst6->sin6_addr, ipaddr, sizeof(ipaddr)));
		}
		else if (dst.ss_family == AF_INET)
		{
			struct sockaddr_in *dst4 = (struct sockaddr_in *) &dst;
			printf("Receive %d bytes %s client %s/%s DESTINATION ADDR:%s\n",
				len, IN6_IS_ADDR_V4MAPPED(&a6->sin6_addr)?"IPv4":"IPv6", host, serv, inet_ntop(AF_INET, &dst4->sin_addr, ipaddr, sizeof(ipaddr)));
		}

		sendto(sock, "HELLO\r\n", 7, 0, (struct sockaddr *)a6, addrlen);
	}

	close(sock);
	return 0;
}

int main(int argc, char *argv[])
{
	int c;
	struct if_nameindex *ifdev, *ic;
	while (1) {
		int option_index = 0;	
		if ((c = getopt_long(argc, argv, "4acmvg:d:i:h:p:l:", NULL, &option_index)) < 0)
			break;
		switch (c) {
		case '4':
			i6af = AF_INET;
			break;			
		case 'a': // auto
			ifdev = if_nameindex();
			for (ic = ifdev; ic->if_index != 0; ic++) {
				if (strcmp(ic->if_name, "eth0") == 0) {
					i6ifindex = ic->if_index;
					strcpy(i6ifname,ic->if_name);
				}
				printf("interface[%d] %s\n", ic->if_index, ic->if_name);
			}
			if_freenameindex(ifdev);
			exit(0);
		case 'g':
			i6grpaddr = strdup(optarg);
			break;
		case 'd':
			strcpy(i6ifname,optarg);
			i6ifindex = if_nametoindex(i6ifname);
			printf("The index of %s is %d\n", i6ifname, i6ifindex);
			break;
		case 'i':
			i6ifindex = atoi(optarg);
			printf("The name of index %d is %s\n", i6ifindex, if_indextoname(i6ifindex, i6ifname));
			exit(0);			
		case 'h':
			i6host = strdup(optarg);
			break;
		case 'p':
			i6port = strdup(optarg);
			break;
		case 'l':
			i6length = atoi(optarg);
			break;
		case 'c':
			i6mode = 1;
			break;
		case 'm':
			i6multicast = 1;
			break;			
		case 'v':
			i6verbose = 1;
			break;
		
		default:
			printf("%s [ -4acmv ] [ -d ifname ] [ -i ifindex ] [ -h host ] [ -p port ] [ -l len ] \n", argv[0]);
			exit(0);
		}
	}

	if (i6mode)
		return i6client(i6host, i6port);

	i6server();
	getchar();

	return 0;
}

