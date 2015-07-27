#include "libapp.h"


int CAppMessage::BuildOnlineMsg(unsigned char **ppData)
{
	int len = 0;
	unsigned char *pNext = NULL;
	unsigned char *pData = (unsigned char *)calloc(1, sizeof(DACMsgHdr)+32);
	DACMsgHdr *pMsg = (DACMsgHdr *) pData;
	pMsg->type = DACMSG_TYPE_ONLINE;
	pMsg->code = 0;
	pNext = pData + sizeof(DACMsgHdr);

	len = IEBuildBype(pNext, IE_APP_STATUS, m_App->m_status);
	pNext += len;

	pMsg->len = pNext - pData;
	*ppData = pData;

	return pMsg->len;
}

int CAppMessage::BuildHBRspMsg(unsigned char *message)
{
	return 0;
}

