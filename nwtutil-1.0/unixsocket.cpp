#include "unixsocket.h"


UnixSocket::UnixSocket(int type)
	:m_sock(-1)
{
	memset (&m_addr, 0, sizeof(m_addr));
	if ((m_sock = socket (AF_LOCAL, type, 0)) < 0) {
		fprintf (stderr, "socket() error: %d (%s)\n", errno, strerror(errno));
		exit (0);
	}
}

UnixSocket::~UnixSocket()
{
	close (m_sock);
}


int  UnixSocket::init(const char* filename)
{
	m_addr.sun_family = AF_LOCAL;
	if (filename) {
		unlink (filename);
		strncpy (m_addr.sun_path, filename, sizeof(m_addr.sun_path)-1);
	}

	if (bind (m_sock, (struct sockaddr* )&m_addr, SUN_LEN(&m_addr)) < 0) {
		fprintf (stderr, "bind() error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}
	
	struct sockaddr_un localaddr;
	socklen_t len = sizeof(localaddr);

	if (getsockname (m_sock, (struct sockaddr* )&localaddr, &len) == 0) {
		printf ("bound name: %s, length: %d\n", localaddr.sun_path, len);
	}
	
	
	return 0;
}

int UnixSocket::start(int (*server) (void* ))
{
	return 0;
}

