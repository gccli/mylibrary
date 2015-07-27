#include "dacqueue.h"

void * DACQueueThread(void *param)
{
	DACQueue *server = (DACQueue *) param;
	server->process();

	return NULL;
}


DACQueue::DACQueue()
	:m_queue_id(-1)
{
}

DACQueue::~DACQueue()
{
}

int DACQueue::init(const char * qname, QUEUE_PROCESS_FUNC func)
{
	struct mq_attr mqattr;
	memset (&mqattr, 0, sizeof(mqattr));
	mqattr.mq_msgsize = sizeof(SAppEvnet);
	mqattr.mq_maxmsg  = 1024;

	mq_unlink(qname);
	mqd_t qid = mq_open(qname, O_RDWR|O_CREAT, 0700, &mqattr);
	if (qid == (mqd_t )-1) {
	  dacerror("failed to open queue %s, error %d %s", qname, errno, strerror(errno));
	  return 1;
	}

	// get attribute
	memset (&mqattr, 0, sizeof(mqattr));
	if (mq_getattr(qid, &mqattr) < 0) {
	  dacerror("failed to get queue attr error %d %s", errno, strerror(errno));
	  return 1;
	}
	debugs("queue(%s) create successfully. max #msgs = %ld, max #bytes/msg = %ld", qname, mqattr.mq_maxmsg, mqattr.mq_msgsize);

	pthread_t th;
	pthread_attr_t thattr;
	pthread_attr_init(&thattr);
	pthread_attr_setdetachstate(&thattr, PTHREAD_CREATE_DETACHED);
	pthread_create(&th, &thattr, DACQueueThread, this);

	m_queue_id = qid;
	m_func = func;

	return 0;
}


void DACQueue::process()
{
	mqd_t qrecv;
	struct mq_attr mqattr;
	unsigned int prio;
	struct timespec ts;

	while (1)
	{
		memset (&mqattr, 0, sizeof(mqattr));
		if (mq_getattr(m_queue_id, &mqattr) == 0) {
			debugs("---- message currently on queue %ld ----", mqattr.mq_curmsgs);
		}

		clock_gettime(CLOCK_REALTIME, &ts);
		ts.tv_sec += 120;
		SAppEvnet ev;
		qrecv = mq_timedreceive(m_queue_id, (char*)&ev, mqattr.mq_msgsize, &prio, &ts);
		if (qrecv < 0) {
			if (errno == ETIMEDOUT || errno == EAGAIN || errno == EINTR)
				continue ;
			dacerror("failed to receive message from queue, error %d %s", errno, strerror(errno));
			break;
		}
		else {
			debugs("get %d bytes message %p from queue %d", ev.msgLen, ev.msgPtr, prio);
			if (m_func)
				m_func(ev.msgPtr, ev.msgLen);
			free(ev.msgPtr);
		}
	}
}


int DACQueue::push(const char *message, int len)
{
	SAppEvnet event;
	memset(&event, 0, sizeof(event));

	event.msgLen = len;
	event.msgPtr = (unsigned char *)calloc(1, len+1);
	memcpy(event.msgPtr, message, len);

    if (mq_send(m_queue_id, (char *) &event, sizeof(SAppEvnet), 0) != 0) {
		dacerror("failed to send msg to queue, error %d %s", errno, strerror(errno));
		return -1;
    }
	return 0;	
}

