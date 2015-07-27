#include "nwt_socket.h"
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>


UDBServer::UDBServer()
	:m_groupaddress(0)
	,m_inputif(0)
	,m_sourcespec(0)
{
}

UDBServer::UDBServer(const char* groupaddr)
	:m_groupaddress(0)
	,m_inputif(0)
	,m_sourcespec(0)
{
	int len = strlen(groupaddr);
	m_groupaddress = new char[len+1];
	strcpy (m_groupaddress, groupaddr);
	m_groupaddress[len] = 0;
}

UDBServer::~UDBServer()
{
}

void UDBServer::setGroupAddr(const char* groupaddr)
{
	strncpy (m_groupaddress, groupaddr, sizeof(m_groupaddress));
}

void UDBServer::setInputIF(const char* iif_addr)
{
	if (m_inputif != NULL) {
		delete [] m_inputif;
		m_inputif = NULL;
	}
	int len = strlen(iif_addr);
	m_inputif = new char[len+1];
	strcpy (m_inputif, iif_addr);
	m_inputif[len] = 0;
}

void UDBServer::setSourceSpec(const char* saddr)
{
	if (m_sourcespec != NULL) {
		delete [] m_sourcespec;
		m_sourcespec = NULL;
	}
	int len = strlen(saddr);
	m_sourcespec = new char[len+1];
	strcpy (m_sourcespec, saddr);
	m_inputif[len] = 0;
}

int UDBServer::start(int port, int mode, int (*server) (void*))
{
	m_port = port;
	struct sockaddr_in addr;
	socklen_t socklen = sizeof(addr);
	memset (&addr, 0, socklen);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(m_port);
	addr.sin_addr.s_addr = m_ipaddress;

	if (bind (sock, (struct sockaddr*) &addr, socklen) < 0) {
		return 1;
	}

	if (mode==MULTICAST_MODE){
		if (m_groupaddress == NULL)
			return 1;
		if (m_sourcespec != NULL){
			if (setMulticastSourceSpec(m_sourcespec, m_groupaddress, m_inputif) != 0)
				return 1;
		} else {
			if (setMulticastGroupAdd(m_groupaddress, m_inputif) != 0)
				return 1;
		}
	}


	if (server) 
		return server (&sock);
	return 0;
}

