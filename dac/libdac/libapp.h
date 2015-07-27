#ifndef __DAC_LIBAPP_H__
#define __DAC_LIBAPP_H__

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <sys/time.h>
#include <pthread.h>

#include "libdac.h"
#include "dacobject.h"
#include "daccommon.h"
#include "daclog.h"

extern const char *DACAppName(int port);

class CAppMessage;
class CAppObject : public DACObject
{
	friend void *DACAppThread(void *);
	friend class CAppMessage;

public:
	CAppObject();
	~CAppObject();

	int init(int myport, const char *dachost, int dacport);

private:
	void process();
	int  sendtodac(const unsigned char *message, int len);
	int  parsemsg(unsigned char *message, int len);

private:
	unsigned int     dacaddrlen;
	struct sockaddr *dacaddr;


private:
	int m_sock;
	CAppMessage *m_msgbuilder;
};


class CAppMessage 
{
public:
	CAppMessage(CAppObject *pObj)
		:m_App(pObj)
	{}
	int BuildOnlineMsg(unsigned char **ppData);
	int BuildHBRspMsg(unsigned char *message);

private:
	CAppObject *m_App;
};


#endif 

