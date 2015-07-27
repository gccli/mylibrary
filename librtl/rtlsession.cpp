#include "rtlayer.h"
#include "rtltimer.h"

#include <assert.h>
#include <map>

typedef std::map< uint32_t, Session * > SessionPool_t;

SessionPool_t session_pool;

Session *SessionCreate()
{
	SessionPool_t::iterator it;
	uint32_t sid = 1;
	for (; ; sid++) {
		it = session_pool.find(sid);
		if(it == session_pool.end())
			break;
	}

	Session *session = (Session *)calloc(1, sizeof(Session));
	session->sid = sid;

	if (rtl_timer_create(&session->timer) != 0)
	{
		free(session);
		return NULL;
	}

	session_pool[sid] = session;

	return session;	
}

Session *SessionLocate(uint32_t sid)
{
	assert(sid > 0);
	SessionPool_t::iterator it = session_pool.find(sid);
	if(it == session_pool.end())
		return NULL;
	

	return it->second;
}

void SessionDelete(Session *session)
{
	SessionPool_t::iterator it = session_pool.begin();
	for (; it != session_pool.end(); ++it)
		if (it->second->sid == session->sid) 
		{
			session_pool.erase(it);
			break;
		}	
}

int SessionSend(Session *pSession, int sock, unsigned char *msg, int len)
{
	struct msghdr msgsend;
	struct iovec  iov[2];


	struct rtlhdr *hdr = (struct rtlhdr *)pSession;
	hdr->seq++;
	hdr->ts = time(NULL);

	iov[0].iov_base = hdr;
	iov[0].iov_len = sizeof(struct rtlhdr);
	iov[1].iov_base = msg;
	iov[1].iov_len = len;

	memset(&msgsend, 0, sizeof(struct msghdr));
	msgsend.msg_iov = iov;
	msgsend.msg_iovlen = 2;
	msgsend.msg_name = pSession->remote_addr;
	msgsend.msg_namelen = pSession->remote_addrlen;

	struct rtlrtt *rtt = &pSession->rtt;
	pSession->rtt_init = 1;
	rtt_init(rtt);
	rtt_newpack(rtt);

	rtl_timer_start(pSession->timer, rtt_start(rtt), 0);

	return sendmsg(sock, &msgsend, 0);	
}

int SessionReSend(Session *session)
{
	struct msghdr msgsend;
	struct iovec  iov[2];



	struct rtlhdr *hdr = (struct rtlhdr *)pSession;
	hdr->seq++;
	hdr->ts = time(NULL);


	iov[0].iov_base = hdr;
	iov[0].iov_len = sizeof(struct rtlhdr);
	iov[1].iov_base = msg;
	iov[1].iov_len = len;

	memset(&msgsend, 0, sizeof(struct msghdr));
	msgsend.msg_iov = iov;
	msgsend.msg_iovlen = 2;
	msgsend.msg_name = pSession->remote_addr;
	msgsend.msg_namelen = pSession->remote_addrlen;

	
}

static void SessionTimeoutHandler(int sig, siginfo_t *si, void *uc)
{
	printf("timeout\n");
}






