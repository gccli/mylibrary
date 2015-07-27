#ifndef THREAD_POOL_SVR_H_
#define THREAD_POOL_SVR_H_

#include <queue>

#include "nwt_socket.h"

class ThreadPoolSvr: public TCPServer
{
friend void* thread_pool(void* );
public:
	ThreadPoolSvr(int thread_pool_size);
	virtual ~ThreadPoolSvr();
	
	int init(int port);
	int start(int(* server)(void *) = NULL);

public:
	int suspend();
	int weakup(int sockfd);

protected:
	std::queue<int>  m_clisock;
	int              m_thread_poolsize;

private:
	pthread_cond_t   m_poolCond;
	pthread_mutex_t  m_poolMutex;
};

#endif 

