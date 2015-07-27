#include "daccenter.h"
#include "dacqueue.h"
#include "net/Tcpserver.h"
#include "ClientConnState.h"

static int qmsgcb(unsigned char *message, int len)
{
    int ret = 0;
    DACMsgHdr *pMsg = (DACMsgHdr *) message;
    switch (pMsg->type)
    {
	case DACMSG_TYPE_ONLINE:
	    debugs("receive client online msg");
	    break;
	case DACMSG_TYPE_HEARTBEAT_RSP:
	    debugs("receive client heartbeat rsp msg");
	    break;
	case DACMSG_TYPE_STATUS_REPORT:
	    debugs("receive client status rsp msg");
	    break;

	default:
	    ret = ERR_INVALID;
	    break;
    }
    return ret;
}

void httpAccept(const CommAcceptCbParams &params)
{
    Connection *serverconn = static_cast<Connection *>(params.data);
    if (params.flag != COMM_OK) {
        // Its possible the call was still queued when the client disconnected
        debugs("accept failure: %s",strerror(params.xerrno));
        return;
    }
    char buf[64] = {0};

    Connection *clientconn = params.conn;
    debugs("Server %s accepted FD:%d",
	   serverconn->local.NtoA(buf,sizeof(buf)), clientconn->fd);
    fd_note(params.conn->fd, "client http connect");
    
    CliConnStat *clistat = new CliConnStat(clientconn);
    clistat->readdata();

/*
    if (s->tcp_keepalive.enabled) {
        commSetTcpKeepalive(params.conn->fd, s->tcp_keepalive.idle, s->tcp_keepalive.interval, s->tcp_keepalive.timeout);
    }
*/
    // Socket is ready, setup the connection manager to start using it
/*    ConnStateData *connState = connStateCreate(params.conn, s);

    typedef CommCbMemFunT<ConnStateData, CommTimeoutCbParams> TimeoutDialer;
    AsyncCall::Pointer timeoutCall =  JobCallback(33, 5,
						  TimeoutDialer, connState, ConnStateData::requestTimeout);
    commSetConnTimeout(params.conn, Config.Timeout.request, timeoutCall);

    connState->readSomeData();
*/
}



int DACcenterInit()
{
    int ret = 0;
    DACQueue *queue = new DACQueue;
    if (queue->init(queue_name, qmsgcb) != 0)
	return 1;

    Connection *conn = new Connection;
    conn->local.SetAnyAddr();
    conn->local.SetPort(atoi(server_port));
    conn->flags = COMM_NONBLOCKING|COMM_DOBIND;
    conn->fd = comm_openex(SOCK_STREAM, IPPROTO_TCP, conn->local, conn->flags, 0, __FUNCTION__);

    // setup the subscriptions such that new connections accepted by listenConn are handled by HTTP
    typedef CommCbFunPtrCallT<CommAcceptCbPtrFun> AcceptCall;
    RefCount<AcceptCall> subCall = commCbCall(5, 5, "httpAccept", CommAcceptCbPtrFun(httpAccept, conn));
    Subscription* sub = new CallSubscription<AcceptCall>(subCall);

    TCPAcceptAdaptor *acceptor = new TCPAcceptAdaptor(conn, sub);
    ret = acceptor->init();

    return ret;
}
