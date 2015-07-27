#include "nwt_socket.h"
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
extern int h_errno;

TCPClient::TCPClient()
{
}

TCPClient::~TCPClient()
{
}

int TCPClient::createlink(const char* dstip, int dstport, int (*interactive) (void* ))
{
	struct sockaddr_in dstaddr;
	memset (&dstaddr, 0, sizeof(dstaddr));

	unsigned long hostip = 0;
	if (getipbyhost(dstip, &hostip) != 0)
		return 1;

	dstaddr.sin_family = AF_INET;
	dstaddr.sin_port   = htons (dstport);
	dstaddr.sin_addr.s_addr = hostip;

	if (::connect (sock, (struct sockaddr* )&dstaddr, sizeof(dstaddr)) < 0) {
		fprintf (stderr, "connect() error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}

	if (interactive) 
		return interactive(this);
	
	return 0;
}

int TCPClient::createlink(const char* dstip, int dstport, int second)
{
	int error = 0, ret;
	struct sockaddr_in dstaddr;
	memset (&dstaddr, 0, sizeof(dstaddr));

	unsigned long hostip = 0;
	struct hostent* host = gethostbyname(dstip);
	if (host != NULL)
		hostip = *(unsigned long *)host->h_addr;
	else {
		fprintf (stderr, "gethostbyname() error: %d (%s)\n", h_errno, hstrerror(h_errno));
		return 1;
	}


	dstaddr.sin_family = AF_INET;
	dstaddr.sin_port   = htons (dstport);
	dstaddr.sin_addr.s_addr = hostip;

	if (setNonblocking() != 0) 
		return 1;
	
	ret = ::connect (sock, (struct sockaddr* )&dstaddr, sizeof(dstaddr));
	if (errno != EINPROGRESS)
		return 1;
	else if (ret == 0) goto done;

	struct timeval tv;
	fd_set rdset, wrset;
	FD_ZERO(&rdset);
	FD_SET(sock, &rdset);
	wrset = rdset;
	tv.tv_sec = second; tv.tv_usec = 0;
	if ((ret = select(sock+1, &rdset, &wrset, NULL, &tv)) == 0) {
		error = ETIMEDOUT; // timeout
	}
	
	if (FD_ISSET(sock, &rdset) || FD_ISSET(sock, &wrset)) {
		getError(&error);
	}

done:
	setNonblocking(false);
	if (error != 0) {
		fprintf (stderr, "createlink() error: %d (%s)\n", error, strerror(error));
		return 1;
	}
	return 0;
}

