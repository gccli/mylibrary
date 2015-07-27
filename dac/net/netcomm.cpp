#include "netcomm.h"
#include "AsyncCall.h"

void comm_callclose_handle(int fd)
{
    fde *F = &fd_table[fd];
    while (F->closeHandler != NULL) {
	AsyncCall::Pointer call = F->closeHandler;
        F->closeHandler = call->Next();
        call->setNext(NULL);
	ScheduleCallHere(call);
    }
}

void comm_close(int fd)
{
    assert(fd >= 0);
    assert(fd < dac_maxfd);

    fde *F = &fd_table[fd];
    if (F->closing())
        return;

    F->flags.close_request = true;

#if USE_SSL
    if (F->ssl) {
	AsyncCall::Pointer startCall=commCbCall(5,4, "commStartSslClose",
                                                FdeCbPtrFun(commStartSslClose, NULL));
        FdeCbParams &startParams = GetCommParams<FdeCbParams>(startCall);
        startParams.fd = fd;
        ScheduleCallHere(startCall);
    }
#endif

    // a half-closed fd may lack a reader, so we stop monitoring explicitly
/*
    if (commHasHalfClosedMonitor(fd))
        commStopHalfClosedMonitor(fd);
    commUnsetFdTimeout(fd);

    // notify read/write handlers after canceling select reservations, if any
    if (COMMIO_FD_WRITECB(fd)->active()) {
	Comm::SetSelect(fd, COMM_SELECT_WRITE, NULL, NULL, 0);
        COMMIO_FD_WRITECB(fd)->finish(COMM_ERR_CLOSING, errno);
    }
    if (COMMIO_FD_READCB(fd)->active()) {
	Comm::SetSelect(fd, COMM_SELECT_READ, NULL, NULL, 0);
        COMMIO_FD_READCB(fd)->finish(COMM_ERR_CLOSING, errno);
    }
*/

    comm_callclose_handle(fd);

/*
    AsyncCall::Pointer completeCall=commCbCall(5,4, "comm_close_complete",
					       FdeCbPtrFun(comm_close_complete, NULL));
    FdeCbParams &completeParams = GetCommParams<FdeCbParams>(completeCall);
    completeParams.fd = fd;
    // must use async call to wait for all callbacks
    // scheduled before comm_close() to finish
    ScheduleCallHere(completeCall);

*/  
}

/// update FD tables after a local or remote (IPC) comm_openex();
void comm_init_opened(const Connection *conn,
		      tos_t tos,
		      const char *note,
		      struct addrinfo *AI)
{
    assert(AI);
    /* update fdstat */

    fd_open(conn->fd, FD_SOCKET, note);

    fde *F = &fd_table[conn->fd];
    F->local_addr = conn->local;
//    F->tosToServer = tos;

    F->sock_family = AI->ai_family;
}

static void comm_set_reuse_addr(int fd)
{
    int on = 1;

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on)) < 0)
        debugs("error in SO_REUSEADDR");
}

static comm_err_t
comm_bind(int s, struct addrinfo &inaddr)
{
    if (bind(s, inaddr.ai_addr, inaddr.ai_addrlen) == 0) {
        debugs("bind socket FD %d", s);
        return COMM_OK;
    }

    debugs("Cannot bind socket FD %d", s);

    return COMM_ERROR;
}

static int comm_apply_flags(int new_socket,
			    Ip::Address &addr,
			    int flags,
			    struct addrinfo *AI)
{
    assert(new_socket >= 0);
    assert(AI);
    const int sock_type = AI->ai_socktype;

    if (!(flags & COMM_NOCLOEXEC))
        fd_set_close_onexec(new_socket);

    if ((flags & COMM_REUSEADDR))
        comm_set_reuse_addr(new_socket);

    if (addr.GetPort() > (unsigned short) 0) {
	//commSetNoLinger(new_socket);
    }

    /* MUST be done before binding or face OS Error: "(99) Cannot assign requested address"... 
    if ((flags & COMM_TRANSPARENT)) {
        comm_set_transparent(new_socket);
    }*/

    if ( (flags & COMM_DOBIND) || addr.GetPort() > 0 || !addr.IsAnyAddr() ) {
        if (comm_bind(new_socket, *AI) != COMM_OK) {
            comm_close(new_socket);
            return -1;
        }
    }

    if (flags & COMM_NONBLOCKING)
        if (fd_set_nonblocking(new_socket) == COMM_ERROR) {
            comm_close(new_socket);
            return -1;
        }
/*
#ifdef TCP_NODELAY
    if (sock_type == SOCK_STREAM)
        commSetTcpNoDelay(new_socket);

#endif

    if (Config.tcpRcvBufsz > 0 && sock_type == SOCK_STREAM)
        commSetTcpRcvbuf(new_socket, Config.tcpRcvBufsz);
*/
    return new_socket;
}

int comm_openex(int sock_type,
		int proto,
		Ip::Address &addr,
		int flags,
		tos_t tos,
		const char *note)
{
    int new_socket;
    struct addrinfo *AI = NULL;

    /* Setup the socket addrinfo details for use */
    addr.GetAddrInfo(AI);
    AI->ai_socktype = sock_type;
    AI->ai_protocol = proto;

    debugs("Attempt open socket for: ");// << addr );

    new_socket = socket(AI->ai_family, AI->ai_socktype, AI->ai_protocol);
    if (new_socket < 0) {
        /* Increase the number of reserved fd's if calls to socket()
         * are failing because the open file table is full.  This
         * limits the number of simultaneous clients */
	debugs("socket failure: ");
	Ip::Address::FreeAddrInfo(AI);
        return -1;
    }

    // XXX: temporary for the transition. comm_openex will eventually have a conn to play with.
    Connection* conn = new Connection;
    conn->local = addr;
    conn->fd = new_socket;

    comm_init_opened(conn, tos, note, AI);
    new_socket = comm_apply_flags(conn->fd, addr, flags, AI);

    Ip::Address::FreeAddrInfo(AI);

    // XXX transition only. prevent conn from closing the new FD on function exit.
    conn->fd = -1;
    delete conn;
    return new_socket;
}

class IoCallback
{
public:
    unsigned char type; // 1:read 2:write
    Connection *conn;
    AsyncCall::Pointer callback;
    char *buf;
    int size;
    int offset;
    int xerrno;

    IoCallback()
	:type(0)
	,conn(NULL)
	,callback(NULL)
	,buf(NULL)
	,size(0)
	,offset(0)
	,xerrno(0)
    {
    }

    void finish(comm_err_t code, int err)
    {
	char ip[64] = {0};
	debugs("called for %s (%d,%d) buf %s", conn->remote.NtoA(ip,sizeof(ip)), code, err, buf);
	assert(callback != NULL);

        typedef CommIoCbParams Params;
        Params &params = GetCommParams<Params>(callback);
        params.fd = conn->fd;
        params.conn = conn;
        params.buf = buf;
        params.size = offset;
        params.flag = code;
        params.xerrno = err;
        ScheduleCallHere(callback);
        callback = NULL;
	reset();    
    }

    void reset()
    {
	conn = NULL;
	buf = NULL;
	xerrno = 0;
    }    
};


/**
 * Attempt a read
 *
 * If the read attempt succeeds or fails, call the callback.
 * Else, wait for another IO notification.
 */
void comm_read_handle(int fd, void *data)
{
    IoCallback *ccb = (IoCallback *) data;
    assert(ccb->type > 0);
    assert(ccb->buf != NULL);
    assert(ccb->size > 0);
    errno = 0;
    int retval;
    retval = read(fd, ccb->buf, ccb->size);
    debugs("FD %d, size %d, retval %d", fd, ccb->size, errno==0?retval:errno);

    if (retval < 0 && !ignore_errno(errno)) {
	debugs("%s", strerror(errno));
        ccb->offset = 0;
        ccb->finish(COMM_ERROR, errno);
        return;
    };

    if (retval >= 0) {
        fd_bytes(fd, retval, FD_READ);
        ccb->offset = retval;
        ccb->finish(COMM_OK, errno);
        return;
    }

    /* Nope, register for some more IO */
    SetSelect(fd, COMM_SELECT_READ, comm_read_handle, data, 0);
}

/**
 * Queue a read. handler/handler_data are called when the read
 * completes, on error, or on file descriptor close.
 */
void comm_read(Connection *conn, char *buf, int size, AsyncCall::Pointer &callback)
{
    debugs("queueing read for ");

    /* Make sure we are open and not closing */
    assert(conn->fd > 0);
    assert(!fd_table[conn->fd].closing());

    IoCallback *ccb = new IoCallback;
    ccb->type = 1;
    ccb->conn = conn;
    ccb->buf = buf;
    ccb->size = size;
    ccb->callback = callback;

    SetSelect(conn->fd, COMM_SELECT_READ, comm_read_handle, ccb, 0);
}
