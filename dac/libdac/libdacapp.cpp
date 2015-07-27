#include "libapp.h"


void *DACAppThread(void *param)
{
	CAppObject *pObj = (CAppObject *) param;
	pObj->process();

	return NULL;
}

CAppObject::CAppObject()
	:DACObject(DAC_OBJ_NONE)
{
}

CAppObject::~CAppObject()
{
}

int CAppObject::init(int myport, const char *dachost, int dacport)
{
	int err;
	char temp[32] = {0};
	sprintf(temp, "%d", myport);

	struct addrinfo hints, *serv6 = NULL, *dac6 = NULL;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET6;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_DGRAM;
	if ((err = getaddrinfo(NULL, temp, &hints, &serv6)) != 0)
	{
		logerror ("getaddrinfo error: %d (%s)", err, gai_strerror(err));
		return 1;
	}

	//===================================================================================
	m_sock = socket(serv6->ai_family, serv6->ai_socktype, 0);
	int enable = 1;
	if (setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) 
	{
		logerror("setsockopt(SO_REUSEADDR) error: %d (%s)", errno, strerror(errno));
		return 1;
	}

	if (bind(m_sock, serv6->ai_addr, serv6->ai_addrlen) < 0)
	{
		logerror("failed to bind ipv6 address: %s", strerror(errno));
		close(m_sock);
		return 1;
	}

	pthread_t th;
	pthread_attr_t thattr;
	pthread_attr_init(&thattr);

	int ret = pthread_create(&th, &thattr, DACAppThread, this);
	if (ret != 0) 
	{
		logerror("failed to create thread");
		return 1;
	}

	m_addr = (struct sockaddr *) calloc(1, serv6->ai_addrlen);
	memcpy(m_addr, serv6->ai_addr, serv6->ai_addrlen);
	m_addrlen = serv6->ai_addrlen;
	freeaddrinfo(serv6);

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET6;
	hints.ai_flags = AI_CANONNAME;
	hints.ai_socktype = SOCK_DGRAM;	
	sprintf(temp, "%d", dacport);
	if ((err = getaddrinfo(dachost, temp, &hints, &dac6)) != 0)
	{
		logerror("getaddrinfo error: %d (%s)", err, gai_strerror(err));
		return 1;
	}
	dacaddr = (struct sockaddr *) calloc(1, dac6->ai_addrlen);
	memcpy(dacaddr, dac6->ai_addr, dac6->ai_addrlen);
	dacaddrlen = dac6->ai_addrlen;
	freeaddrinfo(dac6);

	if (connect(m_sock, dacaddr, dacaddrlen) != 0)
	{
		logwarn("failed to connect dac server");
	}	
	loginfo("app connection to dac started");

	SetName(DACAppName(myport));
	
	m_msgbuilder = new CAppMessage(this);

	unsigned char *message = NULL;
	ret = m_msgbuilder->BuildOnlineMsg(&message);
	sendtodac(message, ret);
	free(message);
	return 0;
}

void CAppObject::process()
{
	struct sockaddr_storage ss;
	unsigned char buffer[1024];

	while(1)
	{
		memset(&ss, 0, sizeof(struct sockaddr_storage));
		socklen_t addrlen = sizeof(ss);
		int len = recvfrom(m_sock, buffer, sizeof(buffer), 0, (struct sockaddr *) &ss, &addrlen);
		if (len < 0) {
			logerror("recvfrom %d error (%d) %s", m_sock, errno, strerror(errno));
			break;
		}
		buffer[len] = 0;
		char host[96] = {0};
		char serv[32] = {0};
		int flag = 0;	
		flag = NI_NUMERICHOST | NI_NUMERICSERV; // tcpdump -nn
		if (getnameinfo((struct sockaddr *) &ss, addrlen, host, sizeof(host), serv, sizeof(serv), flag) != 0)
		{
			logerror("getpeername error %s", strerror(errno));
		}

		loginfo("Receive %d bytes from dac server %s/%s", len, host, serv);
		parsemsg(buffer, len);
	}	
}


int CAppObject::sendtodac(const unsigned char *message, int len)
{
	int slen = sendto(m_sock, message, len, 0, dacaddr, dacaddrlen);
	if (slen > 0) {
		logdebug("%d bytes send to dac server", len);
	} else {
		logerror("send message failure, len %d error %s", slen, strerror(errno));
	}

	return slen;
}


int CAppObject::parsemsg(unsigned char *message, int len)
{
	DACMsgHdr *pMsg = (DACMsgHdr *) message;
	int ret = 0;
	switch(pMsg->type) 
	{
	case DACMSG_TYPE_ONLINE_RSP:
		break;	
	case DACMSG_TYPE_HEARTBEAT:
		break;
	case DACMSG_TYPE_STATUS_REPORT_RSP:
		break;
	default:
		ret = ERR_INVALID;
		break;
	}


	return ret;
}


