#include "epollsvr.h"
#include <string.h>
#include <pthread.h>

#include <arpa/inet.h>
#include <errno.h>
#include <sys/syscall.h>

#include "nwt_config.h"

#define MAX_PACKET_LENGTH 4096

void* thread_accept(void* param);
void* thread_io(void* param);

Epollsvr::Epollsvr()
{
}

Epollsvr::Epollsvr(int port, const char* ip)
	:TCPServer(port, ip)
{
}

Epollsvr::~Epollsvr()
{
	ep_close ();
}

int Epollsvr::ep_create()
{
	epfd = epoll_create(MAX_EPOLL_SIZE);
	if (epfd < 0){
		fprintf (stderr, "epoll_create() error: %d (%s)\n", errno, strerror(errno));
		return -1;
	}

	return 0;
}

int Epollsvr::ep_ctrl(int op, int fd, struct epoll_event* event)
{
	return epoll_ctl(epfd, op, fd, event);
}

int Epollsvr::ep_wait(struct epoll_event * events, int maxevents, int timeout)
{
	return epoll_wait(epfd, events, maxevents, timeout);
}

void Epollsvr::ep_close(void)
{
	close (epfd);
	close_socket();
}

int Epollsvr::ep_accept(struct sockaddr* addr, socklen_t* addrlen)
{
	int clientfd = accept(sock, addr, addrlen);
	if (clientfd < 0) {
		fprintf (stderr, "accept() error: %d (%s)\n", errno, strerror(errno));
		return -1;
	}

	struct epoll_event ev;
	ev.data.fd = clientfd;
	ev.events  = EPOLLIN | EPOLLHUP;
	
	if(ep_ctrl(EPOLL_CTL_ADD, clientfd, &ev) < 0){
		fprintf (stderr, "epoll_ctl(EPOLL_CTL_ADD) error: %d (%s)\n", errno, strerror(errno));
		return -1;
		close (clientfd);
	}

	return clientfd;
}

int Epollsvr::ep_leave(int sockfd)
{
	struct epoll_event ev;
	memset (&ev, 0, sizeof(ev));
	ev.data.fd = sockfd;
	if(ep_ctrl(EPOLL_CTL_DEL, sockfd, &ev) < 0) {
		fprintf (stderr, "epoll_ctl(EPOLL_CTL_DEL) error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}

	close (sockfd);

	return 0;
}

int Epollsvr::init(int port)
{
	if (ep_create() != 0)
		return 1;

	if (TCPServer::init(port) != 0)
		return 1;

	if (setPortReuse() != 0)
		return 1;

	return 0;
}

int Epollsvr::start(int(* server)(void *))
{
	struct epoll_event ev;
	memset (&ev, 0, sizeof(ev));
	ev.data.fd = sock;
	ev.events  = EPOLLIN;

	// register my socket descriptor
	if(ep_ctrl(EPOLL_CTL_ADD, sock, &ev) < 0){
		fprintf (stderr, "epoll_ctl(EPOLL_CTL_ADD) error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}

	struct sockaddr_in cliaddr;
	int maxfds = 0, clientfd;
	struct epoll_event events[MAX_EPOLL_SIZE];
	while(true){
		maxfds = ep_wait(events, MAX_EPOLL_SIZE, -1);// infinitive
		if (maxfds < 0) {
			if (errno == EINTR) continue;
			fprintf (stderr, "epoll_wait() error: %d (%s)\n", errno, strerror(errno));
			break;
		}
		//printf ("active socket descriptor number: %d\n", maxfds);
		for (int i=0; i<maxfds; ++i) {
			if (events[i].data.fd == this->sock) { // for my listen socket
				socklen_t socklen = sizeof (cliaddr);
				memset (&cliaddr, 0, socklen);
				clientfd = ep_accept ((struct sockaddr* )&cliaddr, &socklen);
				if (clientfd > 0) {
					printf("register a connection from \"%s:%d\", socket=%d.\n",
						inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), clientfd);
				}//end if(clientfd ...
			}//end if this->sock ...
			else { // for other listen socket
				do_protocol(events[i].data.fd);
			}
		}//end for
	}//end while

	return 0;
}

static FILE* xfp = NULL;
int Epollsvr::do_protocol(int sockfd)
{
	if (xfp == NULL) {
		xfp = fopen ("/tmp/xchange.out", "w");
	}
	int flags = 0;
	unsigned char buffer[MAX_PACKET_LENGTH];
	int rlen = recv (sockfd, buffer, sizeof(buffer), flags);
	if (rlen > 0) {
		fwrite (buffer, 1, rlen, xfp);
		fflush (xfp);
	}
	else {
		fprintf (stderr, "recv() error: %d (%s)\n", errno, strerror(errno));
		ep_leave(sockfd);
	}
	return rlen;
}

////////////////////////////////////////////////////////////
void* thread_io(void* param)
{
	TEpollsvr* server = (TEpollsvr* )param;
	/*
	struct clisock clientfd;
	if (server->m_clisock.safepop(clientfd) != 0) {
		fprintf (stderr, "client socket queue is empty ?!\n");
		return NULL;
	}
	*/

	int clientfd = 0;
	if (server->m_sock_active.safepop(clientfd) != 0) {
		fprintf (stderr, "active queue is empty!\n");
		return NULL;
	}

	if (server->m_sock_allocated.safepush(clientfd) != 0) {
		fprintf (stderr, "allocated queue is full!\n");
		return NULL;
	}

	int lwpid = gettid();
	printf ("io_thread<%d> created, process sockfd: %d.\n", lwpid, clientfd);

	char tmpbuf[64];
	sprintf (tmpbuf, "/tmp/%d.out", lwpid);
	FILE* fp = fopen (tmpbuf, "w");

	struct sockaddr_in raddr;
	memset (&raddr, 0, sizeof(raddr));
	socklen_t addrlen = sizeof(raddr);
	if (getpeername(clientfd, (struct sockaddr *) &raddr, &addrlen) == 0) 
		;
		
	unsigned char buffer[MAX_PACKET_LENGTH];
	while(true) {
		int len = recv (clientfd, buffer, sizeof(buffer), 0);
		if (len <= 0) {
			break;
		}
		fwrite (buffer, 1, len, fp);
		fflush (fp);
		//PrintPayload(buffer, len, fp);		
	}// end while

	if (server->m_sock_allocated.safeerase(clientfd) != 0) {
		fprintf (stderr, "allocated erase failed!\n");
	}
	server->ep_leave(clientfd);
	if (fp) 
		fclose (fp);
	
	return NULL;
}

TEpollsvr::TEpollsvr()
{
}

TEpollsvr::~TEpollsvr()
{
}
/*
bool TEpollsvr::find_clisock(clisock& clifd)
{
	bool have = false;
	safequeue<clisock>::iterator it = m_clisock.begin();
	for (; it != m_clisock.end(); ++it) {
		if (it->status == 1) {
			have = true;
			clifd  *it;
			it->status = 2;
		}
	}

	return have;
}*/

int TEpollsvr::do_protocol(int sockfd)
{
	/*	struct clisock clientfd;
	clientfd.sock = sockfd;
	if (m_clisock.find(clientfd)) {
		usleep (10);
		return 0;
	}

	clientfd.status = 1;
	if (m_clisock.safepush(clientfd) != 0) {
		fprintf (stderr, "client socket queue is full ?!\n");
		return 1;
	}*/

	if (m_sock_allocated.find(sockfd)) {
		usleep (10);
		return 0;
	}
	if (m_sock_active.find(sockfd)){
		usleep (10);
		return 0;
	}

	if (m_sock_active.safepush(sockfd) != 0) {
		fprintf (stderr, "active queue is empty!\n");
		return 1;
	}

	pthread_t      th;
	pthread_attr_t thattr;
	pthread_attr_init (&thattr);
	pthread_attr_setdetachstate(&thattr, PTHREAD_CREATE_DETACHED);
	size_t stacksize = 1024*1024;
	pthread_attr_setstacksize(&thattr, stacksize);
	pthread_attr_getstacksize(&thattr, &stacksize);
	if (pthread_create(&th, &thattr, thread_io, this) == 0) {
		usleep (10);
		printf ("    thread create success with stack size = %ld\n", stacksize);
		return 0;
	}
	
	return 0;
}


