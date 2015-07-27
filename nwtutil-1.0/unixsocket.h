#ifndef UNIXSOCKET_H_
#define UNIXSOCKET_H_

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>

class UnixSocket
{
public:
	UnixSocket(int type);
	~UnixSocket();

	int  init(const char* filename = NULL);
	int  start(int (*server) (void* ) = NULL);
	
private:
	int m_sock;
	struct sockaddr_un  m_addr;
};

#endif

