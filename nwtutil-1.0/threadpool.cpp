#include <arpa/inet.h>
#include <sys/syscall.h>

#include "threadpool.h"
#include "nwt_config.h"

void* thread_pool(void* param)
{
	ThreadPoolSvr* server = (ThreadPoolSvr* )param;
	int lwpid = gettid();

	char buffer[4096];
	while (!gl_terminate) {
		int clientfd = server->suspend();
		while (true) {
			int len = recv (clientfd, buffer, sizeof(buffer), MSG_DONTWAIT);
			if (len == 0) break;
			if (len < 0) {
				if (errno == EAGAIN) continue;
				if (errno == EINTR)  continue;

				fprintf (stderr, "recv() error: %d, %s\n", errno, strerror(errno));
				break;
			}
		}
		close (clientfd);
		printf ("thread<%d> serve done. socket: %d\n", lwpid, clientfd);
	}
	
	return NULL;
}


ThreadPoolSvr::ThreadPoolSvr(int thread_poolsize)
	:m_thread_poolsize(thread_poolsize)
{
}

ThreadPoolSvr::~ThreadPoolSvr()
{
}

int ThreadPoolSvr::weakup(int sockfd)
{
	pthread_mutex_lock(&m_poolMutex);
	m_clisock.push(sockfd);
	pthread_cond_signal (&m_poolCond);
	pthread_mutex_unlock(&m_poolMutex);

	return 0;
}

int ThreadPoolSvr::suspend()
{
	pthread_mutex_lock(&m_poolMutex);
	while (m_clisock.empty()) {
		pthread_cond_wait(&m_poolCond, &m_poolMutex);
	}

	int clientfd = m_clisock.front();
	m_clisock.pop();
	
	pthread_mutex_unlock(&m_poolMutex);

	return clientfd;
}

int ThreadPoolSvr::init (int port)
{
	int bufsize = 0, mss = 0;
	
	getMSS(&mss);
	getRcvBufSize(sock, &bufsize);
	printf ("default MaxSegmentSize: %d, default RcvBufferSize: %d\n", mss, bufsize);
	bufsize = 512*mss;
	setRcvBufSize(sock, bufsize);
	
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

	pthread_attr_t thattr;
	pthread_attr_init (&thattr);
	pthread_attr_setdetachstate(&thattr, PTHREAD_CREATE_DETACHED);
	size_t stacksize = 1024*1024;
	pthread_attr_setstacksize(&thattr, stacksize);
	
	for (int i=0; i<m_thread_poolsize; ++i) {
		pthread_t th;
		pthread_create(&th, &thattr, thread_pool, this);
		usleep (1);	
	}

	return 0;
}


int ThreadPoolSvr::start(int (*server) (void*))
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

		weakup(rsock);
		
		char straddr[INET_ADDRSTRLEN];
		inet_ntop (AF_INET, &raddr.sin_addr, straddr, sizeof(straddr));
		//printf ("accept connection from: %s:%d, socket descriptor = %d\n", straddr, ntohs(raddr.sin_port), rsock);
	}

	close_socket();
	
	return 0;
}



