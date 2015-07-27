#ifndef EPOLLSVR_H_
#define EPOLLSVR_H_

#include "nwt_socket.h"

#include "safequeue.h"
#include <sys/epoll.h>

#include <queue>

#define MAX_EPOLL_SIZE MAX_FDSIZE
class Epollsvr: public TCPServer
{
public:
	Epollsvr();
	Epollsvr(int port, const char* ip);
	virtual ~Epollsvr();
	
	int init(int port);
	int start(int(* server)(void *) = NULL);

public:
	/**
	 * ����TCP����
	 */
	virtual int ep_accept(struct sockaddr *addr, socklen_t *addrlen);
	/**
	 * ����TCP�ͻ��������ر�
	 */
	virtual int ep_leave(int sockfd);
	/**
	 * ����һ���ض��������ϵ�I/O
	 * @param sockfd: �ͻ���socket������
	 * @return ������ɹ��򷵻�0�����򷵻�����ֵ
	 */
	virtual int do_protocol(int sockfd);

protected:
	int  ep_create();
	int  ep_ctrl(int op, int fd, struct epoll_event *event);
	int  ep_wait(struct epoll_event * events, int maxevents, int timeout);
	void ep_close(void);

protected:
	int		epfd;           // epoll file descriptor
};

struct clisock {
	int    sock;
	int    status; // 0, 1, 2
	clisock ()
		:sock(-1), status(0)
	{}
};

inline bool operator == (const clisock& src, const clisock& dst)
{
	return (src.sock == dst.sock);
}

inline bool operator != (const clisock& src, const clisock& dst)
{	
	return !(src == dst);
}

// multiplex with per-thread
class TEpollsvr : public Epollsvr
{
friend void* thread_io(void* param);
public:
	TEpollsvr();
	virtual ~TEpollsvr() ;

public:
	virtual int do_protocol(int sockfd);

protected:
	bool find_clisock(clisock& clifd);
//	safequeue<clisock> m_clisock;
	safequeue<int> m_sock_active;
	safequeue<int> m_sock_allocated;
};

// multiplex with threadpool
class TPoolEpollsvr : public Epollsvr
{
friend void* thread_pool(void* param);
public:
	TPoolEpollsvr();
	virtual ~TPoolEpollsvr() ;

public:
	virtual int do_protocol(int sockfd);

protected:
	safequeue<int> m_sock_active;
	safequeue<int> m_sock_allocated;
};

#endif 

