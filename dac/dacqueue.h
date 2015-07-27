#ifndef __DAC_QUEUE_H__
#define __DAC_QUEUE_H__

#include <sys/types.h>
#include <mqueue.h>
#include "dac.h"

class DACQueue
{
	friend void * DACQueueThread(void *);

public:
	DACQueue();
	virtual ~DACQueue();

	int init(const char *qname, QUEUE_PROCESS_FUNC func);
	int push(const char *message, int len);

private:
	void process();
	
private:
	mqd_t m_queue_id;
	QUEUE_PROCESS_FUNC m_func;
};

#endif

