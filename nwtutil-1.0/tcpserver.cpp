#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdio.h>

#include "nwt_socket.h"
#include "nwt_config.h"

TCPServer::TCPServer()
{
}

TCPServer::TCPServer(int port, const char * ip)
{
	m_port = port;
	struct sockaddr_in addr;
	if (inet_pton (AF_INET, ip, &addr) == 0)
		m_ipaddress = addr.sin_addr.s_addr;
}

TCPServer::~TCPServer()
{
}

int TCPServer::init (int port)
{
//	int bufsize = 0;
//	getRcvBufSize(sock, &bufsize);
//	setRcvBufSize(sock, 1024*1024);
	m_port = port;
	struct sockaddr_in local;
	socklen_t socklen = sizeof(sockaddr_in);
	memset (&local,  0, socklen);

	local.sin_family = AF_INET;
	local.sin_port = htons(m_port);
	local.sin_addr.s_addr = m_ipaddress;

	if (bind (sock, (struct sockaddr* ) &local, socklen) < 0) {
		fprintf (stderr, "bind() error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}
	
	if (listen (sock, 5) < 0) {
		fprintf (stderr, "listen() error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}

	return 0;
}

int TCPServer::start(int (*server) (void*))
{
	int rsock;
	struct sockaddr_in raddr;
	socklen_t socklen = sizeof(sockaddr_in);
	
	while (true) {
		memset (&raddr,  0, socklen);
		if((rsock = accept (sock, (struct sockaddr*)&raddr, &socklen)) < 0) {
			if (errno == EINTR) {
				printf ("receive INTERRUPT signal.\n");
				sleep (1);
				continue;
			}
			else {
				fprintf (stderr, "accept() error: %d (%s)\n", errno, strerror(errno));
				break;
			}
		}
		char straddr[INET_ADDRSTRLEN];
		inet_ntop (AF_INET, &raddr.sin_addr, straddr, sizeof(straddr));
		printf ("accept connection from: %s:%d, socket descriptor = %d\n", straddr, ntohs(raddr.sin_port), rsock);

		if (server!=NULL && server(&rsock)!=0)
			continue;
	}

	close_socket();
	
	return 0;
}

