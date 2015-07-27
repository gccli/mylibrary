#include "nwt_socket.h"

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>



UDBClient::UDBClient()
	:m_oif(0)
	,m_dstaddr(0)
{
}

UDBClient::~UDBClient()
{	
	if (m_dstaddr != NULL) {
		delete m_dstaddr;
		m_dstaddr = NULL;
	}
	if (m_oif != NULL) {
		delete [] m_oif;
		m_oif = NULL;
	}
}

void UDBClient::setOutputIF(const char* oif)
{
	if (m_oif != NULL) {
		delete [] m_oif;
		m_oif = NULL;
	}

	int len = strlen(oif);
	m_oif = new char[len+1];
	strcpy(m_oif, oif);
	m_oif[len] = 0;
}

int UDBClient::createlink(const char * ip, int port, int (*interactive) (void* ))
{
	if (m_dstaddr != NULL) {
		delete m_dstaddr;
		m_dstaddr = NULL;
	}
	unsigned long hostip = 0;
	if (getipbyhost(ip, &hostip) != 0) {
		return 1;
	}

	struct sockaddr_in addr;
	int dstaddrlen = sizeof(sockaddr_in);
	memset (&addr, 0, dstaddrlen);
	addr.sin_family 	 = AF_INET;
	addr.sin_port		 = htons (port);
	addr.sin_addr.s_addr = hostip;
	m_dstaddr = (struct sockaddr*) malloc(dstaddrlen);
	memcpy (m_dstaddr, &addr, dstaddrlen);

	if ((hostip&0xF0) == 0xE0) { // it a multicast address
		if (setMulticastOIF(m_oif) != 0)
			return 1;
	}		
	else 
	{
		if (::connect (sock, m_dstaddr, dstaddrlen) != 0){
			fprintf (stderr, "connect() error: %d (%s)\n", errno, strerror(errno));
			return 1;
		}
	}

	struct dstinfo dinfo;
	memset(&dinfo, 0, sizeof(dinfo));
	dinfo.sock = sock;
	dinfo.ip   = hostip;
	dinfo.port = ntohs(port);

	if (interactive) interactive(&dinfo);

	return 0;
}

int UDBClient::writedata(const char * buffer,int len,int flags)
{
	return sendto(sock, buffer, len, flags, m_dstaddr, sizeof(sockaddr_in));
}

int UDBClient::sendfile (const char* filename, int count)
{
	int i;
	int fd = open (filename, O_RDONLY);
	if (fd < 0) {
		return 1;
	}
	struct stat st;
	if (fstat(fd, &st) != 0) {
		return 1;
	}

	const int total_filesize = st.st_size;
	const int readlength = 1024;
	int readlen, sendlen, send_totallen=0;

	char buffer[readlength];
	for (i=0; count==0 || i<count; ++i) {
		while ((readlen = read (fd, buffer, readlength)) > 0) {
			if ((sendlen = writedata(buffer, readlen, 0)) < 0) {
				printf ("error send\n");
				send_totallen = 0;
				break;
			}
			send_totallen += sendlen;
		}
		if (send_totallen == total_filesize) {
			send_totallen = 0;
			if (lseek (fd, 0, SEEK_SET) < 0)
				break;
		}
	}
	
	return 0;
}

