#ifndef NET_COMM_H__
#define NET_COMM_H__

#include "daccomm.h"
#include "NetError.h"
#include "CommCalls.h"
#include "fde.h"
#include "fd.h"
#include "Connection.h"
#include "Subscription.h"
#include "Tcpserver.h"
#include "ip/Address.h"

void comm_close(int fd);
int comm_connect_addr(int sock, const Ip::Address &addr);
int comm_openex(int sock_type,
		int proto,
		Ip::Address &addr,
		int flags,
		tos_t tos,
		const char *note);

void comm_read(Connection *conn, char *buf, int size, AsyncCall::Pointer &callback);

int NetModInit();
void SetSelect(int fd, unsigned int type, PF * handler, void *client_data, time_t timeout);
int ignore_errno(int ierrno);




#endif
