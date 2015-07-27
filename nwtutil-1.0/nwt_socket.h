#ifndef NWT_SOCKET_H_
#define NWT_SOCKET_H_

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <pthread.h>

#define gettid() syscall(__NR_gettid)

struct in_pkginfo {
	struct in_addr ipi_addr;
	int            ipi_ifindex;
	int            *flags;
};

struct dstinfo {
	uint32_t ip;
	uint16_t port;
	int      sock;
};

class nwt_socket {
public:
  nwt_socket();
  nwt_socket(int family, int type, int protocol);
  virtual ~nwt_socket();
  
  virtual int readdata(char* pbuf, int len, struct sockaddr* addr, socklen_t* addrlen, struct in_pkginfo* info);
  virtual int readdata(char* pbuf, int len, int flags = 0);
  virtual int writedata(const char* buffer, int len, int flags = 0);

public:
	// options
	int setSockFilter(struct sock_fprog filter);
	int setPortReuse(int enable = 1);
	int setRecvDstAddr(int enable = 1);
	int setRecvIF(int enable = 1);
	int getError(int* error);
	
	int getRcvBufSize(int sockfd, int* size);
	int getSndBufSize(int sockfd, int* size);
	int setRcvBufSize(int sockfd, int  size);
	int setSndBufSize(int sockfd, int  size);

	int setNonblocking(bool on = true);

public:
	int getsockfd() const { return sock; }
	int getifaddr(const char* ifname, char* ipaddr);

public:
	static int getipbyhost(const char* hostname, char* ipaddr);
	static int getipbyhost(const char* hostname, unsigned long* ipaddr);

protected:
	virtual int close_socket();

protected:
  int                 sock;
  int                 m_port;
  unsigned long       m_ipaddress;
};

////////////////////////////////////////////////////////////////////////////////
// RAW Socket
class RAWSocket: public nwt_socket
{
public:
	RAWSocket(int protocol);
	virtual ~RAWSocket();

public:
	int buildRawUDPData(const char* srcip, int srcport, const char* destip, int destport, const char* payload, int len);
	int buildTcpSYN(const char* srcip, int srcport, const char* destip, int destport);
	
protected:
	int setHeaderIncl();
};

////////////////////////////////////////////////////////////////////////////////
// TCP
class TCPSocket : public nwt_socket
{
public:
	TCPSocket();
	virtual ~TCPSocket();

	int getMSS(int* mss); // TCP_MAX_SEG
	int setMSS(int mss);

	// Window size, SO_RCVBUF
};

class TCPServer: public TCPSocket
{
public:
	TCPServer();
	TCPServer(int port, const char* ip);
	virtual ~TCPServer();

public:
	virtual int init (int port);
	virtual int start(int (*server) (void*) = NULL);

private:
};

class TCPClient: public TCPSocket
{
public:
	TCPClient();
	virtual ~TCPClient();

	int createlink(const char* dstip, int dstport, int second); // non-blocking
	int createlink(const char* dstip, int dstport, int (*interactive) (void* ) = NULL);	
};


////////////////////////////////////////////////////////////////////////////////
// UDP
class UDPSocket : public nwt_socket
{
public:
  UDPSocket();
  virtual ~UDPSocket();

	// IP Multicast Options
  int setMulticastTTL(int ttl);
  int setMulticastLoopback(int enable);
  int setMulticastOIF(const char* ifname);
	
  int setMulticastGroupAdd(const char* group_address, const char* input_interface = NULL);
  int setMulticastSourceSpec(const char* source_address, const char* group_address, const char* input_interface = NULL);	
};

#define        UNICAST_MODE     0
#define        MULTICAST_MODE   1
class UDBServer: public UDPSocket
{
public:
	UDBServer();
	UDBServer(const char* group_address);
	virtual ~UDBServer();

public:
	void setGroupAddr(const char* groupaddr);
	void setInputIF(const char* iif_addr);
	void setSourceSpec(const char* saddr);
	
	int  start(int port, int mode, int (*server) (void*));
	
private:
	char* m_groupaddress;
	char* m_inputif;
	char* m_sourcespec;
};

class UDBClient: public UDPSocket
{
public:
	UDBClient();
	virtual ~UDBClient();

	void setOutputIF(const char* oif);
	int  createlink (const char* ip, int port, int (*interactive) (void* ) = NULL);
	int  writedata(const char * buffer,int len,int flags = 0);
	int  sendfile (const char* filename, int count);

private:
	char*              m_oif;
	struct sockaddr*   m_dstaddr;
};

#endif // NWT_SOCKET_H_

