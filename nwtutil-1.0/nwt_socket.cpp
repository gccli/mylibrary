#include <ctype.h>

#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netdb.h>
#include <fcntl.h>
#include <linux/filter.h>

#include "logger.h"
#include "nwt_socket.h"
#include "nwt_config.h"

nwt_socket::nwt_socket()
	:sock(-1)
	,m_port(0)
	,m_ipaddress(INADDR_ANY)
{
}

nwt_socket::nwt_socket(int family, int type, int protocol)
	:sock(-1)
	,m_port(0)
	,m_ipaddress(INADDR_ANY)
{
	sock = socket (family, type, protocol);
	if (sock < 0) {
		fprintf (stderr, "socket() error: %d (%s)\n", errno, strerror(errno));
		exit(1);
	}
}

nwt_socket::~nwt_socket()
{
	if (sock > 0) {
		close_socket(); 
		sock = -1;
	}
}

int nwt_socket::close_socket()
{
	return close (sock);;
} 

int nwt_socket::readdata(char* pbuf, int len, struct sockaddr* addr, socklen_t* addrlen, struct in_pkginfo* info)
{
	struct msghdr msg;
	memset (&msg, 0, sizeof(msg));
	struct iovec iov = {pbuf, len};
	int n, flags = *(info->flags);

	struct cmsghdr* pcmhdr;
	union {
		struct cmsghdr cm;
		char           control[CMSG_SPACE(sizeof(struct in_addr))+CMSG_SPACE(sizeof(struct in_pkginfo))];
	}control_un;

	msg.msg_control    = control_un.control;
	msg.msg_controllen = sizeof(control_un.control);
	msg.msg_flags = 0;

	msg.msg_name    = addr;
	msg.msg_namelen = *addrlen;
	msg.msg_iov     = &iov;
	msg.msg_iovlen  = 1;

	if ((n = recvmsg(sock, &msg, flags)) < 0) {
		fprintf (stderr, "recvmsg() error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}

	for (pcmhdr = CMSG_FIRSTHDR(&msg); pcmhdr != NULL; pcmhdr = CMSG_NXTHDR(&msg, pcmhdr)) {
		
	}
	
	return n;
}

int nwt_socket::readdata(char* pbuf, int len, int flags)
{
	return recv(sock, pbuf, len, flags);
}

int nwt_socket::writedata(const char* buffer, int len, int flags)
{
	return send (sock, buffer, len, flags);
}

int nwt_socket::setSockFilter(struct sock_fprog filter)
{
	if (setsockopt (sock, SOL_SOCKET, SO_ATTACH_FILTER, &filter,  sizeof(filter)) < 0) {
		fprintf (stderr, "setsockopt(SO_ATTACH_FILTER) error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}

	return 0;
}

int nwt_socket::setPortReuse(int enable)
{
	if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		fprintf (stderr, "setsockopt(SO_REUSEADDR) error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}
	
	return 0;
}

int nwt_socket::setRecvDstAddr(int enable)
{
#ifdef IP_RECVDSTADDR
	if (setsockopt (sock, IPPROTO_IP, IP_RECVDSTADDR, &enable, sizeof(int)) < 0) {
		fprintf (stderr, "setsockopt(IP_RECVDSTADDR) error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}
#endif
	return 0;
}

int nwt_socket::setRecvIF(int enable)
{
#ifdef IP_RECVIF
	if (setsockopt (sock, IPPROTO_IP, IP_RECVIF, &enable, sizeof(int)) < 0) {
		fprintf (stderr, "setsockopt(IP_RECVIF) error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}
#endif

	return 0;
}

int nwt_socket::getError(int* error)
{
	socklen_t len = sizeof(*error);
	return getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len);
}

int nwt_socket::getRcvBufSize(int sockfd,int* size)
{
	socklen_t len = sizeof(int);
	if (getsockopt (sockfd, SOL_SOCKET, SO_RCVBUF, size, &len) < 0) {
		fprintf (stderr, "getsockopt(SO_RCVBUF) error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}
	
	return 0;
}

int nwt_socket::getSndBufSize(int sockfd,int* size)
{
	socklen_t len = sizeof(int);
	if (getsockopt (sockfd, SOL_SOCKET, SO_SNDBUF, size, &len) < 0){
		fprintf (stderr, "getsockopt(SO_SNDBUF) error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}

	return 0;
}

int nwt_socket::setRcvBufSize(int sockfd,int size)
{
	if (setsockopt (sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(int)) < 0) {
		fprintf (stderr, "setsockopt(SO_RCVBUF) error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}
	return 0;
}

int nwt_socket::setSndBufSize(int sockfd,int size)
{
	if (setsockopt (sockfd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(int)) < 0) {
		fprintf (stderr, "setsockopt(SO_SNDBUF) error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}
	return 0;
}

int nwt_socket::setNonblocking(bool on)
{
	int flags = 0;
	if ((flags = fcntl (sock, F_GETFL, 0)) < 0) {
		fprintf (stderr, "fcntl(F_GETFL) error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}

	if (on)
		flags |= O_NONBLOCK;
	else 
		flags &= ~O_NONBLOCK;

	if ((flags = fcntl (sock, F_SETFL, flags)) < 0) {
		fprintf (stderr, "fcntl(F_SETFL) error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}
	
	return 0;
}

// ifname is eth0, for example
int nwt_socket::getifaddr(const char* ifname, char* ipaddr)
{
	if (sock < 0) {
	  printf ("socket file descriptor not initialize.\n");
	  return 1;
	}
	if (ipaddr == NULL)
		return 1;

	struct ifreq ifr;
	memset (&ifr, 0, sizeof (ifr));
	strcpy (ifr.ifr_name, ifname);
	
	if (ioctl (sock, SIOCGIFADDR, &ifr) < 0) {
		fprintf (stderr, "ioctl(SIOCGIFADDR) error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}
	struct sockaddr_in addr;
	memcpy (&addr, &(ifr.ifr_addr), sizeof(addr));
	inet_ntop (AF_INET, &(addr.sin_addr), ipaddr, INET_ADDRSTRLEN);
	
	return 0;
}

int nwt_socket::getipbyhost(const char* hostname, char* ipaddr)
{
	struct hostent* host = gethostbyname(hostname);
	if (host != NULL)
		strcpy (ipaddr, host->h_addr);
	else {
		fprintf (stderr, "gethostbyname() error: %d (%s)\n", h_errno, hstrerror(h_errno));
		return 1;
	}

	return 0;
}

int nwt_socket::getipbyhost(const char* hostname, unsigned long* ipaddr)
{
	struct hostent* host = gethostbyname(hostname);
	if (host != NULL)
		*ipaddr = *(unsigned long *)host->h_addr;
	else {
		fprintf (stderr, "gethostbyname() error: %d (%s)\n", h_errno, hstrerror(h_errno));
		return 1;
	}

	return 0;
}


/**
 * UDP Socket
 */

UDPSocket::UDPSocket()
	:nwt_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)
{
}

UDPSocket::~UDPSocket()
{
}

int UDPSocket::setMulticastTTL(int ttl)
{
	if (setsockopt (sock, IPPROTO_IP, IP_MULTICAST_TTL, (char* )&ttl, sizeof(int))){
		fprintf (stderr, "setsockopt(IP_MULTICAST_TTL) error: %d (%s)\n", errno, strerror(errno));
		return 1;			
	}

	return 0;
}

int UDPSocket::setMulticastLoopback(int enable)
{
	if (setsockopt (sock, IPPROTO_IP, IP_MULTICAST_LOOP, (char*) &enable, sizeof(int)))	{
		fprintf (stderr, "setsockopt(IP_MULTICAST_LOOP) error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}

	return 0;
}

// @param: eth0 for example
int UDPSocket::setMulticastOIF(const char* ifname)
{
	char ipaddr[20] = "\0";
	if (getifaddr(ifname, ipaddr) != 0)
		return 1;
	

	struct in_addr addr;
	addr.s_addr =  inet_addr (ipaddr);
	if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, (char* ) &addr, sizeof(struct in_addr))){
		fprintf (stderr, "setsockopt(IP_MULTICAST_IF) error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}
	return 0;
}


int UDPSocket::setMulticastGroupAdd(const char* group_address, const char* input_interface)
{
	struct ip_mreq mreq;
	memset(&mreq, 0, sizeof(ip_mreq));	
	mreq.imr_multiaddr.s_addr = inet_addr(group_address);
	if (input_interface != NULL)	{ // -i—°œÓ 
		char ipaddr[20] = "\0";
		getifaddr(input_interface, ipaddr);
		mreq.imr_interface.s_addr = inet_addr(ipaddr);
	} else	{				
		mreq.imr_interface.s_addr = INADDR_ANY; 		
	}
	if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,(char*)&mreq,sizeof(mreq)) == -1) {
		fprintf (stderr, "setsockopt(IP_ADD_MEMBERSHIP) error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}

	return 0;
}

int UDPSocket::setMulticastSourceSpec(const char* source_address, const char* group_address, const char* input_interface)
{
	struct ip_mreq_source src_group;
	memset (&src_group, 0, sizeof (ip_mreq_source));
	src_group.imr_multiaddr.s_addr  = inet_addr(group_address);
	src_group.imr_sourceaddr.s_addr = inet_addr(source_address);
	if (input_interface != NULL) {
		char ipaddr[20] = "\0";
		getifaddr(input_interface, ipaddr);
		src_group.imr_interface.s_addr = inet_addr(ipaddr);

	} else {
		src_group.imr_interface.s_addr = INADDR_ANY;
	}

	if (setsockopt(sock, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, (char* )&src_group, sizeof(src_group))) {
		fprintf (stderr, "setsockopt(IP_ADD_SOURCE_MEMBERSHIP) error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}

	return 0;
}

/**
 * TCP sockets
 */
TCPSocket::TCPSocket()
	:nwt_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)
{

}

TCPSocket::~TCPSocket()
{

}

int TCPSocket::getMSS(int* mss)
{
	socklen_t len = sizeof(int);
	if (getsockopt (sock, IPPROTO_TCP, TCP_MAXSEG, mss, &len) < 0) {
		fprintf (stderr, "getsockopt(TCP_MAXSEG) error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}

	return 0;
}

int TCPSocket::setMSS(int mss)
{
	if (setsockopt (sock, IPPROTO_TCP, TCP_MAXSEG, &mss, sizeof(int)) < 0) {
		fprintf (stderr, "getsockopt(TCP_MAXSEG) error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}
	
	return 0;
}

